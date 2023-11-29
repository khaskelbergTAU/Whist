package il.arazim.plugins.user

import il.arazim.concurrent.Uploader
import il.arazim.getExecutablesDir
import il.arazim.getRunResults
import il.arazim.getSelected
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
import java.io.InputStream
import kotlin.io.path.exists
import kotlin.io.path.readText
import kotlin.io.path.walk
import kotlin.io.path.writeText

fun Application.configureUserRouting() {
    install(AutoHeadResponse)
    install(StatusPages) {
        exception<ParameterException> { call, cause ->
            call.respondText(
                status = HttpStatusCode.BadRequest,
                text = "Invalid parameters: ${cause.params.joinToString(", ")}. Message: ${cause.message}"
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

                if (!botName.matches("[A-Za-z0-9_]+".toRegex())) {
                    call.respondText(
                        status = HttpStatusCode.BadRequest,
                        text = "Bot name contains illegal characters. Only latin letters, numbers and underscores are accepted"
                    )
                    return@post
                }

                val group = call.principal<GroupPrincipal>()?.name
                if (group == null) {
                    call.respond(UnauthorizedResponse())
                    return@post
                }

                val (success, output) = Uploader.getInstance(group).uploadBot(botName, fileStream)

                if (!success) {
                    call.respondText(status = HttpStatusCode.UnprocessableEntity, text = "Compilation error: $output")
                } else {
                    call.respondOk()
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
                get("/bots") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@get
                    }

                    val allCompilations = getExecutablesDir(group).toFile().list()
                        .filterNot { it.endsWith(".failed") }

                    val selected = getSelected(group).readText()

                    call.respondText("""
                    {
                    "unselected": [${allCompilations.filterNot { it == selected }.joinToString(separator = "\", \"", prefix = "\"", postfix = "\"")}],
                    "selected": "$selected"
                    }
                    """.trimIndent())
                }
                post("select") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@post
                    }

                    val botName = call.request.queryParameters["bot"] ?: throw ParameterException("bot", "Bot name is missing")

                    if (!getExecutablesDir(group).resolve(botName).exists()) {
                        call.respond(status = HttpStatusCode.BadRequest, """
                            Bot doesn't exist
                        """.trimIndent())
                    }

                    getSelected(group).writeText(botName)

                    call.respondOk()
                }
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