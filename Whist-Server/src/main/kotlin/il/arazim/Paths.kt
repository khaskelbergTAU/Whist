package il.arazim

import java.nio.file.Path
import kotlin.io.path.*

val rootDir = "serverFiles".toPath()
val apiDir = "API".toPath()
fun getSubmissionDir(group: String): Path = rootDir.resolve("bots/$group/submissions").apply { createDirectories() }
fun getSourceDir(group: String): Path = getSubmissionDir(group).resolve("source").apply { createDirectories() }
fun getCompilationLogsDir(group: String): Path = getSubmissionDir(group).resolve("clogs").apply { createDirectories() }
fun getExecutablesDir(group: String): Path = getSubmissionDir(group).resolve("exec").apply { createDirectories() }
fun getSelected(group: String): Path =
    getSubmissionDir(group).resolve("latest.txt").also { if (!it.exists()) it.createFile() }

fun getAllBots(group: String) : List<String> =
    (getExecutablesDir(group).toFile().list()?.asList() ?: emptyList<String>()) + (getExecutablesDir("common").toFile()
        .list()?.asList()?: emptyList())

fun getWrapper(): Path = apiDir.resolve("API.c")

fun getRunDir(group: String): Path = rootDir.resolve("run/$group").apply { createDirectories() }