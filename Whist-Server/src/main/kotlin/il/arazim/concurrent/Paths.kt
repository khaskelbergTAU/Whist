package il.arazim.concurrent


import il.arazim.rootDir
import java.nio.file.Path
import kotlin.io.path.Path
import kotlin.io.path.createDirectories


fun getSubmissionDir(group: String): Path = rootDir.resolve("bots/$group/submissions").apply { createDirectories() }
fun getSourceDir(group: String): Path = il.arazim.il.arazim.getSubmissionDir(group).resolve("source").apply { createDirectories() }
fun getCompilationLogsDir(group: String): Path = il.arazim.il.arazim.getSubmissionDir(group).resolve("clogs").apply { createDirectories() }
fun getExecutablesDir(group: String): Path = il.arazim.il.arazim.getSubmissionDir(group).resolve("exec").apply { createDirectories() }
fun getLatest(group: String): Path = il.arazim.il.arazim.getSubmissionDir(group).resolve("latest.txt")


fun getWrapper() = Path("./API/API.c")