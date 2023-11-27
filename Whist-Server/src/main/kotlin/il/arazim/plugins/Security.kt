package il.arazim.plugins

import il.arazim.toPath
import io.ktor.server.application.*
import io.ktor.server.auth.*
import java.nio.file.Path
import java.security.MessageDigest
import kotlin.text.Charsets.UTF_8

fun Application.configureSecurity() {
    authentication {
        val userRealm = "user-access"
        val userTable: Map<String, ByteArray> = mapOf(
            "group1" to "test1",
            "group2" to "test2",
            "group3" to "test3",
            "group4" to "test4",
        ).mapValues { (user, password) ->
            MessageDigest.getInstance("MD5").digest("$user:$userRealm:$password".toByteArray(UTF_8))
        }

        digest("user-auth") {
            realm = userRealm
            digestProvider { userName, _ ->
                userTable[userName]
            }
            validate { credentials ->
                GroupPrincipal(credentials.userName)
            }
        }
    }
}

data class GroupPrincipal(val name: String) : Principal {
    val botsDir: Path
        get() = "bots/$name/".toPath()
}