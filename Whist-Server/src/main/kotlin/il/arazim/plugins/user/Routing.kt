package il.arazim.plugins.user

import il.arazim.*
import il.arazim.concurrent.Runner
import il.arazim.concurrent.Uploader
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
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@get
                    }

                    val results =
                        getRunDir(group).resolve("stdout.txt").takeIf { it.exists() }?.readText(Charsets.UTF_8)

                    if (results == null) {
                        throw Exception("There has been no runs")
                    }

                    call.respondText(results)
                }
                get("/errors") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@get
                    }

                    val results =
                        getRunDir(group).resolve("stderr.txt").takeIf { it.exists() }?.readText(Charsets.UTF_8)

                    if (results == null) {
                        throw Exception("There has been no runs")
                    }

                    call.respondText(results)
                }
                get("/log/{index}") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@get
                    }

                    val index = call.parameters["index"]?.toIntOrNull()?.takeIf { it in 1..4 }
                        ?: throw ParameterException("index", "Index missing or invalid.. (Index should be from 1 to 4)")

                    val log =
                        getRunDir(group).resolve("$index.txt").takeIf { it.exists() }?.readText(Charsets.UTF_8)

                    if (log == null) {
                        throw Exception("There has been no runs")
                    }

                    call.respondText(log)
                }
                get("/bots") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@get
                    }

                    val allCompilations = getAllBots(group)
                        .filterNot { it.endsWith(".failed") }

                    val selected = getSelected(group).readText()

                    call.respondText(
                        """
                        {
                        "unselected": [${
                            allCompilations.filterNot { it == selected }
                                .joinToString(separator = "\", \"", prefix = "\"", postfix = "\"")
                        }],
                        "selected": "$selected"
                        }
                        """.trimIndent()
                    )
                }
                post("/select") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@post
                    }

                    val botName =
                        call.request.queryParameters["bot"] ?: throw ParameterException("bot", "Bot name is missing")

                    if (botName !in getAllBots(group)) {
                        call.respond(
                            status = HttpStatusCode.BadRequest, """
                            Bot doesn't exist
                            """.trimIndent()
                        )
                    }

                    getSelected(group).writeText(botName)

                    call.respondOk()
                }
                post("/start") {
                    val group = call.principal<GroupPrincipal>()?.name
                    if (group == null) {
                        call.respond(UnauthorizedResponse())
                        return@post
                    }

                    val bots =
                        call.request.queryParameters["bots"]?.split(",")?.map(String::trim)?.takeIf { it.size == 4 }
                            ?: throw ParameterException("bots", "Bot list missing or invalid")

                    val allBots = getAllBots(group)

                    bots.find {
                        it !in allBots
                    }?.let { throw ParameterException("bots", "Bot doesn't exist: $it") }

                    val rounds = call.request.queryParameters["run_count"]?.toIntOrNull()
                        ?: throw ParameterException("run_count", "Invalid run count")

                    Runner.getInstance(group).newRun(bots, rounds)

                    call.respondOk()
                }
            }
            post("/logout") {
                call.respondRedirect("/")
            }
            staticFiles(
                remotePath = "/",
                dir = "zips".toPath().toFile(),
                index = null
            )
        }
    }
}

private suspend fun ApplicationCall.respondOk() {
    this.respondText(status = HttpStatusCode.OK, text = "ok")
}

class ParameterException(message: String, vararg params: String) : Exception(message) {
    val params = setOf(params)
}