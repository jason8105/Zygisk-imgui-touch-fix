plugins {
    id("com.android.application")
}

android {
    namespace = "com.zygisk.imgui"
    compileSdk = 33

    defaultConfig {
        applicationId = "com.zygisk.imgui"
        minSdk = 24
        targetSdk = 33
        versionCode = 1
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
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

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.10.1")
}

// Custom Gradle task to assemble the flashable Magisk Zygisk module zip
tasks.register("assembleMagiskModule") {
    dependsOn("assembleRelease")
    doLast {
        val outputDir = layout.buildDirectory.dir("magisk_module").get().asFile
        if (outputDir.exists()) outputDir.deleteRecursively()
        outputDir.mkdirs()

        // Create Magisk module structure
        val zygiskDir = File(outputDir, "zygisk")
        zygiskDir.mkdirs()

        // Copy compiled libraries for supported ABIs
        val abis = listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
        for (abi in abis) {
            val soFile = File(layout.buildDirectory.get().asFile, "intermediates/cmake/release/obj/$abi/libzygisk.so")
            if (soFile.exists()) {
                val targetAbiDir = File(zygiskDir, abi)
                targetAbiDir.mkdirs()
                soFile.copyTo(File(targetAbiDir, "libzygisk.so"), overwrite = true)
            }
        }

        // Write module.prop for Magisk 24-26 compatibility
        File(outputDir, "module.prop").writeText(
            """
            id=zygisk_imgui_touch_fix
            name=Zygisk ImGui Universal Touch Fix
            version=v1.0
            versionCode=1
            author=Elite Engineer
            description=Universal ImGui Menu with Zygisk and robust touch input redirection supporting Magisk 24-26.
            minMagisk=24000
            """.trimIndent()
        )

        // Create empty customize.sh and service.sh if needed
        File(outputDir, "customize.sh").writeText("#!/system/bin/sh\nui_print \"Installing Zygisk ImGui Touch Fix...\"\n")

        // Package into ZIP
        val zipFile = File(layout.buildDirectory.get().asFile, "zygisk_imgui_module.zip")
        if (zipFile.exists()) zipFile.delete()

        ant.invokeMethod("zip", mapOf("destfile" to zipFile, "basedir" to outputDir))
        println("Magisk module successfully created at: ${zipFile.absolutePath}")
    }
}

tasks.named("assemble") {
    finalizedBy("assembleMagiskModule")
}
