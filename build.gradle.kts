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

    val cxxOutputDir = layout.buildDirectory.dir("intermediates/cmake/release/obj")
    from(cxxOutputDir) {
        include("**/libzygisk.so")
    }
    into(layout.buildDirectory.dir("magisk_module/zygisk"))
    
    // Ensure task correctly tracks inputs/outputs to prevent Gradle validation errors
    val soFiles = fileTree(cxxOutputDir) {
        include("**/libzygisk.so")
    }
    inputs.file(soFiles)
    outputs.dir(layout.buildDirectory.dir("magisk_module/zygisk"))
}

val buildMagiskZip by tasks.registering(Zip::class) {
    dependsOn(prepareMagiskModule)
    archiveFileName.set("Zygisk-ImGui-Touch-Fix.zip")
    destinationDirectory.set(layout.buildDirectory.dir("outputs/magisk"))
    
    val moduleDir = layout.buildDirectory.dir("magisk_module")
    from(moduleDir)
    from("module.prop")
    from("customize.sh")
}

tasks.named("assemble") {
    finalizedBy(buildMagiskZip)
}
