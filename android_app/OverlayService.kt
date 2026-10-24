package com.cheat.overlay

import android.app.*
import android.content.Context
import android.content.Intent
import android.graphics.*
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.view.View
import android.view.WindowManager
import org.json.JSONArray
import java.net.HttpURLConnection
import java.net.URL

class OverlayService : Service() {
    private lateinit var windowManager: WindowManager
    private lateinit var overlayView: OverlayView

    override fun onCreate() {
        super.onCreate()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel("1", "Overlay", NotificationManager.IMPORTANCE_LOW)
            getSystemService(NotificationManager::class.java).createNotificationChannel(channel)
        }
        val notification = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, "1")
        } else {
            Notification.Builder(this)
        }.setContentTitle("PUBG Overlay").setSmallIcon(android.R.drawable.ic_menu_info_details).build()

        startForeground(1, notification)

        windowManager = getSystemService(Context.WINDOW_SERVICE) as WindowManager
        overlayView = OverlayView(this)
        
        val params = WindowManager.LayoutParams(
            WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.MATCH_PARENT,
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY else WindowManager.LayoutParams.TYPE_PHONE,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE or WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
            PixelFormat.TRANSLUCENT
        )
        windowManager.addView(overlayView, params)
    }

    override fun onDestroy() {
        super.onDestroy()
        if (::overlayView.isInitialized) windowManager.removeView(overlayView)
    }
    override fun onBind(intent: Intent?): IBinder? = null
}

class OverlayView(context: Context) : View(context) {
    private var boxes: List<Box> = emptyList()
    private val paint = Paint().apply { color = Color.RED; style = Paint.Style.STROKE; strokeWidth = 5f }
    private val textPaint = Paint().apply { color = Color.RED; textSize = 40f; isAntiAlias = true }
    private val handler = Handler(Looper.getMainLooper())

    init {
        startPolling()
    }

    private fun startPolling() {
        handler.postDelayed(object : Runnable {
            override fun run() {
                Thread {
                    try {
                        val url = URL("http://127.0.0.1:8080/get_boxes")
                        val conn = url.openConnection() as HttpURLConnection
                        conn.connectTimeout = 100
                        conn.readTimeout = 100
                        val json = conn.inputStream.bufferedReader().readText()
                        val arr = JSONArray(json)
                        val newBoxes = mutableListOf<Box>()
                        for (i in 0 until arr.length()) {
                            val obj = arr.getJSONObject(i)
                            newBoxes.add(Box(obj.getDouble("x").toFloat(), obj.getDouble("y").toFloat(),
                                             obj.getDouble("w").toFloat(), obj.getDouble("h").toFloat(),
                                             obj.getString("text")))
                        }
                        boxes = newBoxes
                        postInvalidate()
                    } catch (e: Exception) { }
                }.start()
                handler.postDelayed(this, 50)
            }
        }, 0)
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        for (box in boxes) {
            // Автоматически подстраиваем координаты под реальный размер экрана
            val scaleX = width.toFloat() / 3048.0f
            val scaleY = height.toFloat() / 2032.0f
            
            val left = box.x * scaleX
            val top = box.y * scaleY
            val right = (box.x + box.w) * scaleX
            val bottom = (box.y + box.h) * scaleY

            canvas.drawRect(left, top, right, bottom, paint)
            canvas.drawText(box.text, left, top - 10, textPaint)
        }
    }
}

data class Box(val x: Float, val y: Float, val w: Float, val h: Float, val text: String)
