package il.arazim.concurrent

import il.arazim.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.NonCancellable
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import kotlinx.coroutines.yield
import java.io.InputStream
import java.nio.file.Path
import kotlin.io.path.*

class Uploader private constructor(private val group: String) {
    private val mutex = Mutex()

    suspend fun uploadBot(name: String, bytes: InputStream) = withContext(NonCancellable + Dispatchers.IO) {
        val savePath: Path
        val botName: String
        mutex.withLock {
            var resolveOverride = 0
            while (getSourceDir(group).resolve("${name}${if (resolveOverride != 0) "_$resolveOverride" else ""}.c")
                    .exists()) resolveOverride++
            botName = "${name.toPath().normalize().fileName}${if (resolveOverride != 0) "_$resolveOverride" else ""}"
            savePath = getSourceDir(group).resolve("$botName.c")
        }
        savePath.writeBytes(bytes.readBytes())

        val stdOutPath = getCompilationLogsDir(group).resolve("$botName-stdout").normalize()
        val stdErrPath = getCompilationLogsDir(group).resolve("$botName-stderr").normalize().toFile()

        val compiler = ProcessBuilder(
            "gcc",
            "-I", apiDir.absolutePathString(),
            savePath.absolutePathString(),
            getWrapper().absolutePathString(),
            "-o", getExecutablesDir(group).resolve(botName).absolutePathString()
        )
            .redirectOutput(stdOutPath.toFile())
            .redirectError(stdErrPath)
            .start()

        while (compiler.isAlive) yield()

        if (compiler.exitValue() != 0) {
            getExecutablesDir(group).resolve("$botName.failed").createFile()
        }

        val success = compiler.exitValue() == 0

        return@withContext success to (if (success) stdOutPath.readText() else stdErrPath.readText())
    }

    companion object {
        private val INSTANCES = mutableMapOf<String, Uploader>()
        private val mutex = Mutex()

        suspend fun getInstance(group: String) = mutex.withLock {
            return@withLock INSTANCES[group] ?: Uploader(group)
        }
    }
}