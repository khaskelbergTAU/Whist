package il.arazim

import il.arazim.plugins.admin.configureAdminAuthentication
import il.arazim.plugins.common.configureHTTP
import il.arazim.plugins.user.configureUserAuthentication
import il.arazim.plugins.user.configureUserRouting
import io.ktor.server.application.*

fun main(args: Array<String>) {
    io.ktor.server.netty.EngineMain.main(args)
}

fun Application.userModule() {
    configureUserAuthentication()
    configureHTTP()
    configureUserRouting()
}

fun Application.adminModule() {
    configureAdminAuthentication()
    configureHTTP()
    configureUserRouting()
}