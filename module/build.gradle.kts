plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.imguitouchfix"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DANDROID_STL=c++_shared"
            }
        }
        
        ndk {
            abiFilters.addAll(setOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64"))
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }
}

tasks.register<Zip>("packageZygiskModule") {
    dependsOn("assembleRelease")
    archiveFileName.set("zygisk-imgui-touch-fix.zip")
    destinationDirectory.set(layout.buildDirectory.dir("outputs/magisk"))

    val compiledDir = layout.buildDirectory.dir("intermediates/cmake/release/obj")
    
    doFirst {
        val baseDir = layout.buildDirectory.dir("tmp/magisk_staging").get().asFile
        if (baseDir.exists()) baseDir.deleteRecursively()
        baseDir.mkdirs()

        // Create module structure
        file("$baseDir/META-INF/com/google/android").mkdirs()
        file("$baseDir/zygisk").mkdirs()

        // module.prop
        file("$baseDir/module.prop").writeText(
            """
            id=zygisk_imgui_touch_fix
            name=Zygisk ImGui Universal Touch Fix
            version=v1.0.0
            versionCode=100
            author=EliteEngineer
            description=Universal ImGui menu overlay with robust touch input interception for all game engines (Unity, Unreal, Native).
            minMagisk=24000
            """.trimIndent()
        )

        // customize.sh & service.sh if needed
        file("$baseDir/customize.sh").writeText("SKIPUNZIP=0\n")

        // Copy shared libraries into zygisk/<abi>/libzygisk.so
        val objDir = compiledDir.get().asFile
        if (objDir.exists()) {
            objDir.listFiles()?.forEach { abiDir ->
                if (abiDir.isDirectory) {
                    val targetAbiDir = file("$baseDir/zygisk/${abiDir.name}")
                    targetAbiDir.mkdirs()
                    abiDir.listFiles { _, name -> name == "libzygisk.so" }?.forEach { lib ->
                        lib.copyTo(file("${targetAbiDir.absolutePath}/libzygisk.so"), overwrite = true)
                    }
                }
            }
        }
        
        from(baseDir)
    }
}

tasks.named("assembleRelease").configure {
    finalizedBy("packageZygiskModule")
}
