package il.arazim.concurrent

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
    private val _latest = MutableStateFlow(getSavedLatest())
    val latest
        get() = _latest.asStateFlow()

    private val latestPath = getSourceDir(group).resolve("latest")

    private fun getSavedLatest() = getLatest(group).readText(Charsets.UTF_8)
    private fun saveLatest() = getLatest(group).writeText(latest.value)

    suspend fun uploadBot(name: String, bytes: InputStream) = withContext(NonCancellable + Dispatchers.IO) {
        val savePath: Path
        val botName: String
        mutex.withLock {
            var resolveOverride = 0
            while (getSourceDir(group).resolve("${name}_$resolveOverride").exists()) resolveOverride++
            botName = "${name}_$resolveOverride"
            savePath = getSourceDir(group).resolve(botName)
            latestPath.deleteIfExists()
            latestPath.createSymbolicLinkPointingTo(savePath)
        }
        savePath.writeBytes(bytes.readBytes())

        val compiler = ProcessBuilder("gcc ${savePath.absolutePathString()} ${getWrapper().absolutePathString()} -o ${getExecutablesDir(group).resolve(botName)}")
            .redirectOutput(getCompilationLogsDir(group).resolve("$botName-stdout").toFile())
            .redirectError(getCompilationLogsDir(group).resolve("$botName-stderr").toFile()).start()

        while (compiler.isAlive) yield()

        if (compiler.exitValue() != 0) {
            getExecutablesDir(group).resolve("$botName.failed").createFile()
        }
    }

    companion object {
        private val INSTANCES = mutableMapOf<String, Uploader>()
        private val mutex = Mutex()

        suspend fun getInstance(group: String) = mutex.withLock {
            return@withLock INSTANCES[group] ?: Uploader(group)
        }
    }
}