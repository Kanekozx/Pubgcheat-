package com.cheat.overlay

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.Settings
import android.widget.Button
import android.widget.LinearLayout
import android.app.Activity

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
        val btnStop = Button(this).apply {
            text = "STOP OVERLAY"
            setOnClickListener {
                stopService(Intent(this@MainActivity, OverlayService::class.java))
            }
        }
        layout.addView(btnStart)
        layout.addView(btnStop)
        setContentView(layout)
    }
}
