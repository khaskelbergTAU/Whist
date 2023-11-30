package il.arazim.plugins.user

import il.arazim.toPath
import io.ktor.server.application.*
import io.ktor.server.auth.*
import java.nio.file.Path
import java.security.MessageDigest
import kotlin.text.Charsets.UTF_8

fun Application.configureUserAuthentication() {
    authentication {
        val userRealm = "user-access"
        val userTable: Map<String, ByteArray> = mapOf(
            "admin" to "OmriYuvalYuvalMark",
            "Pythagoras" to "a^2+b^2=c^2",
            "Euclid" to "e^(pi*i)=-1",
            "Newton" to "E=mc^2",
            "Fibonacci" to "f(n)=f(n-1)+f(n-2)",
            "Ferma" to "Password didn't fit in the margin",
            "test" to "test",
            "Anna Zaks" to "0525381648",
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
        get() = "./server_files/bots/$name/".toPath()
}