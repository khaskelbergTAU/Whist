package il.arazim.concurrent

import il.arazim.apiDir
import il.arazim.getExecutablesDir
import il.arazim.getRunDir
import io.ktor.util.logging.*
import kotlinx.coroutines.*
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlin.io.path.*

class Runner private constructor(private val group: String) {

    private val mutex = Mutex()
    private var currentJob: Job? = null

    @OptIn(ExperimentalPathApi::class)
    suspend fun newRun(bots: List<String>, rounds: Int, coroutineScope: CoroutineScope) {
        assert(bots.size == 4)

        val findBot = { name: String ->
            if (getExecutablesDir(group).resolve(name).exists())
                getExecutablesDir(group).resolve(name)
            else
                getExecutablesDir("common").resolve(name)
        }

        mutex.withLock {
            currentJob?.takeIf { it.isActive }?.cancelAndJoin()

            val runDir = getRunDir(group).apply {
                deleteRecursively()
                createDirectories()
            }

            runDir.resolve("bots.txt").writeText(bots.joinToString(separator = "\n"))

            currentJob = coroutineScope.launch(context = Dispatchers.IO) {
                val process = ProcessBuilder(
                    apiDir.resolve("grader").absolutePathString(),
                    findBot(bots[0]).absolutePathString(),
                    findBot(bots[1]).absolutePathString(),
                    findBot(bots[2]).absolutePathString(),
                    findBot(bots[3]).absolutePathString(),
                    runDir.resolve("1.txt").absolutePathString(),
                    runDir.resolve("2.txt").absolutePathString(),
                    runDir.resolve("3.txt").absolutePathString(),
                    runDir.resolve("4.txt").absolutePathString(),
                    rounds.toString()
                )
                    .also { LOGGER.info("Running as $group: ${it.command().joinToString(separator = " ")}") }
                    .redirectError(runDir.resolve("stderr.txt").toFile())
                    .redirectOutput(runDir.resolve("stdout.txt").toFile())
                    .start()
                try {
                    while (process.isAlive) {
                        yield()
                        if (!isActive) LOGGER.warn("$currentJob DIED!!!!!")
                    }
                } finally {
                    LOGGER.debug(
                        "Job {} finalising. Active: {}. Process: PID {}, Alive: {}",
                        currentJob,
                        isActive,
                        process.pid(),
                        process.isAlive
                    )
                    if (process.isAlive) process.destroy()
                    if (process.isAlive) process.destroyForcibly()
                }
            }
            LOGGER.debug("Job {} launched.", currentJob)
        }
    }


    companion object {
        private val INSTANCES = mutableMapOf<String, Runner>()
        private val mutex = Mutex()

        private val LOGGER = KtorSimpleLogger("il.arazim.Runner")

        suspend fun getInstance(group: String) = mutex.withLock {
            return@withLock INSTANCES[group] ?: Runner(group).also { INSTANCES[group] = it }
        }
    }
}