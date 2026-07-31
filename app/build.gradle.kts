import java.io.File

plugins {
    id("com.android.application")
}

android {
    namespace = "com.zygisk.imgui"
    compileSdk = 34

    defaultConfig {
        minSdk = 24
        targetSdk = 34
        versionCode = 100
        versionName = "1.0.0"

        ndk {
            abiFilters.addAll(setOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64"))
        }

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17", "-frtti", "-fexceptions", "-Wall")
                arguments("-DANDROID_STL=c++_static")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
        debug {
            isMinifyEnabled = false
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

// Custom Gradle task to pack compiled binaries into standard Magisk module zip layout
tasks.register<Zip>("packageMagiskModule") {
    dependsOn("externalNativeBuildRelease")

    archiveFileName.set("zygisk-imgui-menu.zip")
    destinationDirectory.set(file("${rootProject.projectDir}/out"))

    // Add module metadata template files
    from(file("${rootProject.projectDir}/template"))

    // Copy compiled native library into zygisk/<abi>/libzygisk.so
    val abis = listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
    abis.forEach { abi ->
        val cmakeOutDir = file("${rootProject.projectDir}/out/zygisk/$abi")
        if (cmakeOutDir.exists()) {
            from(cmakeOutDir) {
                into("zygisk/$abi")
            }
        }
    }

    doFirst {
        file("${rootProject.projectDir}/out").mkdirs()
    }
}

tasks.named("assembleRelease") {
    finalizedBy("packageMagiskModule")
}
