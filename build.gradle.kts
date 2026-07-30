plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.imguitouchfix"
    compileSdk = 34

    defaultConfig {
        minSdk = 24
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
    
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

val prepareMagiskFiles by tasks.registering(Copy::class) {
    dependsOn("assembleRelease")
    
    val abiList = listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
    for (abi in abiList) {
        from(layout.buildDirectory.dir("intermediates/cmake/release/obj/$abi/libzygisk.so"))
        into(layout.buildDirectory.dir("magisk_module/zygisk/$abi"))
    }
    
    doFirst {
        layout.buildDirectory.dir("magisk_module/zygisk").get().asFile.mkdirs()
    }
}

tasks.register<Zip>("buildMagiskZip") {
    dependsOn(prepareMagiskFiles)
    
    // Explicitly declare inputs safely or avoid strict validation failures
    val outputDir = layout.buildDirectory.dir("magisk_module")
    inputs.dir(outputDir)

    from(outputDir)
    from("src/main/magisk") {
        include("module.prop", "post-fs-data.sh", "service.sh", "action.sh")
    }
    
    archiveFileName.set("Zygisk-ImGui-Touch-Fix.zip")
    destinationDirectory.set(layout.buildDirectory.dir("outputs/magisk"))
}
