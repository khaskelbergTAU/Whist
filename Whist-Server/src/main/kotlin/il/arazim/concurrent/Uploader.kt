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
import il.arazim.toPath

class Uploader private constructor(private val group: String) {
    private val mutex = Mutex()
    private val _latest = MutableStateFlow(getSavedLatest())
    val latest
        get() = _latest.asStateFlow()

//    private val latestPath = getSourceDir(group).resolve("latest")

    private fun getSavedLatest() = getLatest(group)

        .readText(Charsets.UTF_8).takeIf { it != "" && getExecutablesDir(group).resolve(it).exists() }
    private fun saveLatest() = latest.value?.let { getLatest(group).writeText(it, Charsets.UTF_8) }

    suspend fun uploadBot(name: String, bytes: InputStream) = withContext(NonCancellable + Dispatchers.IO) {
        val savePath: Path
        val botName: String
        mutex.withLock {
            var resolveOverride = 0
            while (getSourceDir(group).resolve("${name}_$resolveOverride").exists()) resolveOverride++
            botName = "${name.toPath().normalize().fileName}_$resolveOverride"
            savePath = getSourceDir(group).resolve("$botName.c")
//            latestPath.deleteIfExists()
//            latestPath.createSymbolicLinkPointingTo(savePath)
        }
        savePath.writeBytes(bytes.readBytes())

        val compiler = ProcessBuilder("gcc ${savePath.absolutePathString()} ${
            getWrapper().absolutePathString()} -o ${getExecutablesDir(group).resolve(botName)}")
            .redirectOutput(getCompilationLogsDir(group).resolve("$botName-stdout").normalize().toFile())
            .redirectError(getCompilationLogsDir(group).resolve("$botName-stderr").normalize().toFile()).start()

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