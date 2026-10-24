package com.cheat.overlay

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.Settings
import android.widget.Button
import android.widget.LinearLayout

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (!Settings.canDrawOverlays(this)) {
            val intent = Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION, Uri.parse("package:$packageName"))
            startActivity(intent)
        }
        val layout = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL }
        val btnStart = Button(this).apply {
            text = "START OVERLAY"
            setOnClickListener {
                startForegroundService(Intent(this@MainActivity, OverlayService::class.java))
                finish()
            }
        }
        layout.addView(btnStart)
        setContentView(layout)
    }
}
