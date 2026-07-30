plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.imguitouchfix"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34

        ndk {
            abiFilters.addAll(setOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64"))
        }

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
            }
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

val prepareMagiskModule by tasks.registering(Copy::class) {
    dependsOn("assembleRelease")
    
    val outputDir = layout.buildDirectory.dir("magisk_module")
    val intermediatesDir = layout.buildDirectory.dir("intermediates/cmake/release/obj")

    doFirst {
        outputDir.get().asFile.deleteRecursively()
    }

    from(file("src/main/magisk")) {
        include("**/*")
    }
    
    // Safely copy compiled shared libraries from CMake output if they exist
    into(outputDir)
    
    doLast {
        val magiskDir = outputDir.get().asFile
        val zygiskDir = file("$magiskDir/zygisk")
        zygiskDir.mkdirs()

        val abis = listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
        for (abi in abis) {
            val libFile = file("${intermediatesDir.get().asFile}/$abi/libzygisk.so")
            if (libFile.exists()) {
                val targetAbiDir = file("$zygiskDir/$abi")
                targetAbiDir.mkdirs()
                libFile.copyTo(file("$targetAbiDir/libzygisk.so"), overwrite = true)
            }
        }
    }
}

tasks.register<Zip>("buildMagiskZip") {
    dependsOn(prepareMagiskModule)
    archiveFileName.set("Zygisk-ImGui-Touch-Fix-Release.zip")
    destinationDirectory.set(layout.buildDirectory.dir("outputs/zip"))
    from(layout.buildDirectory.dir("magisk_module"))
}
