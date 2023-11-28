package il.arazim.user.plugins

<<<<<<< HEAD
=======
import il.arazim.concurrent.Uploader
import il.arazim.concurrent.getRunResults
>>>>>>> c77bb08 (Added mechanisms for concurrent bot uploading and compiling)
import io.ktor.http.*
import io.ktor.http.content.*
import io.ktor.server.application.*
import io.ktor.server.auth.*
import io.ktor.server.http.content.*
import io.ktor.server.plugins.autohead.*
import io.ktor.server.plugins.statuspages.*
import io.ktor.server.request.*
import io.ktor.server.response.*
import io.ktor.server.routing.*
<<<<<<< HEAD
=======
import io.ktor.util.*
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
>>>>>>> c77bb08 (Added mechanisms for concurrent bot uploading and compiling)
import java.io.InputStream
import kotlin.io.path.*

fun Application.configureRouting() {
    install(AutoHeadResponse)
    install(StatusPages) {
        exception<ParameterException> { call, cause ->
            call.respondText(
                status = HttpStatusCode.BadRequest,
                text = "Invalid form parameters: ${cause.params.joinToString(", ")}. Message: ${cause.message}"
            )
        }
        exception<Throwable> { call, cause ->
            call.respondText(text = "500: $cause", status = HttpStatusCode.InternalServerError)
        }
    }
    routing {
        authenticate("user-auth") {
            staticResources("/", basePackage = "static/pages")
            post("/upload") {
                val multipart = call.receiveMultipart()

                var botNameForm: String? = null
                var fileStreamForm: InputStream? = null

                multipart.forEachPart { part ->
                    when (part) {
                        is PartData.FileItem -> {
                            if (part.name == "bot_file") {
                                fileStreamForm = part.streamProvider()
                            }
                        }

                        is PartData.FormItem -> {
                            if (part.name == "bot_name") {
                                botNameForm = part.value
                            }
                        }

                        is PartData.BinaryChannelItem -> throw Exception("Binary channels are unsupported")
                        is PartData.BinaryItem -> throw Exception("Binary items are unsupported")
                    }
                }

                val botName = botNameForm ?: throw ParameterException("bot_name", "Bot name is missing")
                val fileStream = fileStreamForm ?: throw ParameterException("bot_file", "Bot file is missing")

<<<<<<< HEAD
                if (!botName.matches("[A-Za-z0-9_]+".toRegex())) throw ParameterException(
                    "bot_name",
                    "Bot name contains illegal characters"
                )

                val botsDir = call.principal<GroupPrincipal>()?.botsDir

                if (botsDir == null) {
                    call.respond(HttpStatusCode.Unauthorized)
                    return@post
                }

                val submissionDir = botsDir.resolve("submissions/$botName").apply { createDirectories() }

                var order = 0
                while (submissionDir.resolve("$order.c").exists()) order++

                val botFile = submissionDir.resolve("$order.c").apply {
                    createFile()
                }
                botFile.writeBytes(fileStream.readBytes())

                submissionDir.resolve("latest.c").apply {
                    deleteIfExists()
                    createSymbolicLinkPointingTo(botFile)
                }

                call.respondOk()


=======
                call.respondOk()

                val group = call.principal<GroupPrincipal>()?.name
                if (group == null) {
                    call.respond(UnauthorizedResponse())
                    return@post
                }

                coroutineScope {
                    launch {
                        Uploader.getInstance(group).uploadBot(botName, fileStream)
                    }
                }
            }
            route("/run") {
                get("/results") {
                    val results = getRunResults().readText(Charsets.UTF_8)
                    
                    if (results == "") {
                        throw Exception("There has been no runs")
                    }
                    
                    call.respondText(results)
                }
>>>>>>> c77bb08 (Added mechanisms for concurrent bot uploading and compiling)
            }
            post("/logout") {
                call.respondRedirect("/")
            }
        }
    }
}

private suspend fun ApplicationCall.respondOk() {
    this.respondText(status = HttpStatusCode.OK, text = "ok")
}

class ParameterException(message: String, vararg params: String) : Exception(message) {
    val params = setOf(params)
}