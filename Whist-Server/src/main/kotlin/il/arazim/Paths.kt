package il.arazim

import java.nio.file.Path
import kotlin.io.path.*

val rootDir = "serverFiles".toPath()
val apiDir = "API".toPath()
fun getSubmissionDir(group: String): Path = rootDir.resolve("bots/$group/submissions").apply { createDirectories() }
fun getSourceDir(group: String): Path = getSubmissionDir(group).resolve("source").apply { createDirectories() }
fun getCompilationLogsDir(group: String): Path = getSubmissionDir(group).resolve("clogs").apply { createDirectories() }
fun getExecutablesDir(group: String): Path = getSubmissionDir(group).resolve("exec").apply { createDirectories() }
fun getLatest(group: String): Path = getSubmissionDir(group).resolve("latest.txt").also { if (!it.exists()) it.createFile() }

fun getWrapper() = apiDir.resolve("API.c")

fun getRunDir(): Path = rootDir.resolve("run").apply { createDirectories() }
fun getRunResults(): Path = getRunDir().resolve("output.txt").also { if (!it.exists()) it.createFile() }
fun getRunLog(group: String): Path = getRunDir().resolve("$group.log").also { if (!it.exists()) it.createFile() }