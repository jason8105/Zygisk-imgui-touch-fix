plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.imguitouchfix"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34
        consumerProguardFiles("consumer-rules.pro")
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17 -frtti -fexceptions"
                arguments += "-DANDROID_STL=c++_shared"
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    ndkVersion = "25.2.9519653"
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
}

val createMagiskZip = tasks.register("createMagiskZip") {
    dependsOn("assembleRelease")
    doLast {
        val buildDir = layout.buildDirectory.get().asFile
        val intermediatesDir = File(buildDir, "intermediates")
        
        // Find compiled libzygisk.so across possible outputs
        val possibleSoFiles = listOf(
            File(buildDir, "intermediates/merged_native_libs/release/out/lib"),
            File(buildDir, "intermediates/cmake/release/obj"),
            File(buildDir, "intermediates/stripped_native_libs/release/out/lib")
        )

        var sourceLibDir: File? = null
        for (dir in possibleSoFiles) {
            if (dir.exists()) {
                sourceLibDir = dir
                break
            }
        }

        val outputDir = File(buildDir, "magisk_module")
        if (outputDir.exists()) outputDir.deleteRecursively()
        outputDir.mkdirs()

        // Copy module configuration and scripts
        val rootDirProject = rootProject.projectDir
        File(rootDirProject, "module.prop").copyTo(File(outputDir, "module.prop"), overwrite = true)
        val customizeSh = File(rootDirProject, "customize.sh")
        if (customizeSh.exists()) {
            customizeSh.copyTo(File(outputDir, "customize.sh"), overwrite = true)
        }

        // Setup zygisk ABI folders
        val zygiskDir = File(outputDir, "zygisk")
        zygiskDir.mkdirs()

        val abiMap = mapOf(
            "arm64-v8a" to "arm64-v8a",
            "armeabi-v7a" to "armeabi-v7a",
            "x86" to "x86",
            "x86_64" to "x86_64"
        )

        var copiedAny = false
        if (sourceLibDir != null) {
            abiMap.forEach { (gradleAbi, folderName) ->
                val abiFolder = File(sourceLibDir, gradleAbi)
                val soFile = File(abiFolder, "libzygisk.so")
                if (soFile.exists()) {
                    val targetFolder = File(zygiskDir, folderName)
                    targetFolder.mkdirs()
                    soFile.copyTo(File(targetFolder, "libzygisk.so"), overwrite = true)
                    println("Successfully packed ${soFile.absolutePath} into zygisk/$folderName/")
                    copiedAny = true
                }
            }
        }

        if (!copiedAny) {
            // Fallback: check build/outputs/cmake
            val cmakeOutputDir = File(buildDir, "outputs/cmake")
            if (cmakeOutputDir.exists()) {
                cmakeOutputDir.walkTopDown().forEach { file ->
                    if (file.name == "libzygisk.so") {
                        val parentAbi = file.parentFile?.name ?: "arm64-v8a"
                        val targetFolder = File(zygiskDir, parentAbi)
                        targetFolder.mkdirs()
                        file.copyTo(File(targetFolder, "libzygisk.so"), overwrite = true)
                        println("Successfully packed fallback ${file.absolutePath} into zygisk/$parentAbi/")
                        copiedAny = true
                    }
                }
            }
        }

        if (!copiedAny) {
            throw GradleException("Failed to locate compiled libzygisk.so for any ABI!")
        }

        // Package into flashable zip
        val zipFile = File(buildDir, "zygisk_imgui_touch_fix_universal.zip")
        if (zipFile.exists()) zipFile.delete()

        ant.invokeMethod("zip", mapOf("destFile" to zipFile, "basedir" to outputDir))
        println("Magisk module zip successfully created at: ${zipFile.absolutePath}")
    }
}

tasks.named("build") {
    finalizedBy(createMagiskZip)
}
