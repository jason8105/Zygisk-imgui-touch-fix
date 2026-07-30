plugins {
    id("com.android.library")
}

android {
    namespace = "com.zygisk.touchfix"
    compileSdk = 34

    defaultConfig {
        minSdk = 26
        targetSdk = 34

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")
        
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
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

dependencies {
    implementation("androidx.appcompat:appcompat:1.6.1")
}

val createMagiskModuleZip by tasks.registering(Zip::class) {
    dependsOn("assembleRelease")
    archiveFileName.set("Zygisk-Touch-Fix-Module.zip")
    destinationDirectory.set(file("$buildDir/outputs/magisk"))

    val stagingDir = file("$buildDir/staging")
    doFirst {
        stagingDir.deleteRecursively()
        stagingDir.mkdirs()

        // Create standard Magisk module structure
        file("$stagingDir/META-INF/com/google/android").mkdirs()
        
        // Copy module configuration
        if (file("src/main/magisk/module.prop").exists()) {
            file("src/main/magisk/module.prop").copyTo(file("$stagingDir/module.prop"))
        }
        if (file("src/main/magisk/customize.sh").exists()) {
            file("src/main/magisk/customize.sh").copyTo(file("$stagingDir/customize.sh"))
        }

        // Copy compiled native libraries for Zygisk (zygisk/arm64-v8a.so, etc.)
        val libsDir = file("$buildDir/intermediates/merged_native_libs/release/out/lib")
        if (libsDir.exists()) {
            libsDir.copyRecursively(file("$stagingDir/zygisk"), true)
        }
    }

    from(stagingDir)
}

tasks.named("build") {
    dependsOn(createMagiskModuleZip)
}
