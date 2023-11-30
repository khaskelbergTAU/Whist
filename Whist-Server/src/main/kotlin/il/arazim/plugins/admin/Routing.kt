package il.arazim.plugins.admin

import il.arazim.*
import il.arazim.concurrent.Uploader
import io.ktor.http.*
import io.ktor.server.application.*
import io.ktor.server.response.*
import io.ktor.server.routing.*
import io.ktor.util.logging.*
import io.ktor.util.pipeline.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.NonCancellable
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import kotlin.io.path.*

fun Application.configureAdminRouting() {

}

val LOGGER = KtorSimpleLogger("il.arazim.plugins.admin.Routing")


val mutex = Mutex()

@OptIn(ExperimentalPathApi::class)
suspend fun PipelineContext<Unit, ApplicationCall>.handleAdminUpload() {
    LOGGER.info("Admin upload detected")

    mutex.withLock {
        withContext(Dispatchers.IO + NonCancellable) {
            getSubmissionDir("admin").takeIf { it.exists() }?.deleteRecursively()
            val groups = rootDir.resolve("bots").toFile().list()?.filter { it !in listOf("common", "admin") }

            if (groups == null) {
                throw Exception("No groups exist")
            }

            val selected = groups.associateWith { group ->
                getSelected(group).readText()
            }.onEach { (group, bot) ->
                if (bot != "") LOGGER.debug("Group {} selected bot {}", group, bot)
                else LOGGER.warn("Group {} didn't select any bot!!!", group)
            }.filterValues { it != "" }

            val findBot = { group: String, name: String ->
                if (getExecutablesDir(group).resolve(name).exists())
                    getExecutablesDir(group).resolve(name)
                else
                    getExecutablesDir("common").resolve(name)
            }

            selected.forEach { (group, bot) ->
                val botFile = findBot(group, bot)

                botFile.copyTo(getExecutablesDir("admin").resolve(group))

                LOGGER.debug("{}: {} - copied", group, bot)
            }
        }
    }

    call.respondOk()
}

private suspend fun ApplicationCall.respondOk() {
    this.respondText(status = HttpStatusCode.OK, text = "ok")
}