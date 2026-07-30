plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.imgui"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += listOf("-DANDROID_STL=c++_shared")
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

val createMagiskModuleZip by tasks.registering(Zip::class) {
    dependsOn("assembleRelease")
    archiveFileName.set("zygisk-imgui-release.zip")
    destinationDirectory.set(layout.buildDirectory.dir("outputs/magisk"))

    val cmakeOutputDir = layout.buildDirectory.dir("intermediates/cmake/release/obj")
    
    // Explicitly declare inputs and outputs to prevent Gradle validation task errors
    val abis = listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
    for (abi in abis) {
        val soFile = cmakeOutputDir.map { it.dir("$abi/libzygisk.so").asFile }
        inputs.file(soFile).optional()
        from(soFile) {
            into("zygisk/$abi")
        }
    }

    from("module.prop")
    from("customize.sh") {
        into("")
        optional()
    }
}

tasks.named("assemble") {
    finalizedBy(createMagiskModuleZip)
}
