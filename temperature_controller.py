"""
Temperature Controller App for ESP8266-based Motor Control
Allows setting temperature thresholds and monitoring sensor data
"""

import tkinter as tk
from tkinter import messagebox, ttk
import requests
import threading
from datetime import datetime
import json

class TemperatureControllerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Temperature Motor Controller")
        self.root.geometry("600x500")
        
        # Configuration
        self.esp_ip = "192.168.1.100"  # Change to your ESP8266 IP
        self.esp_port = 80
        self.base_url = f"http://{self.esp_ip}:{self.esp_port}"
        
        # Current values
        self.current_temp = tk.DoubleVar(value=0.0)
        self.current_humidity = tk.DoubleVar(value=0.0)
        self.motor_status = tk.StringVar(value="OFF")
        self.threshold_temp = tk.DoubleVar(value=28.0)
        
        self.setup_ui()
        self.update_data()
    
    def setup_ui(self):
        """Setup the GUI components"""
        # Title
        title = tk.Label(self.root, text="🌡️ Temperature Motor Controller", 
                         font=("Arial", 16, "bold"))
        title.pack(pady=10)
        
        # ESP8266 Connection Frame
        conn_frame = ttk.LabelFrame(self.root, text="ESP8266 Connection", padding=10)
        conn_frame.pack(fill="x", padx=10, pady=5)
        
        tk.Label(conn_frame, text="ESP8266 IP:").grid(row=0, column=0, sticky="w")
        ip_entry = tk.Entry(conn_frame)
        ip_entry.insert(0, self.esp_ip)
        ip_entry.grid(row=0, column=1, sticky="ew")
        
        def update_ip():
            self.esp_ip = ip_entry.get()
            self.base_url = f"http://{self.esp_ip}:{self.esp_port}"
            messagebox.showinfo("Success", f"IP updated to {self.esp_ip}")
        
        tk.Button(conn_frame, text="Update IP", command=update_ip).grid(row=0, column=2, padx=5)
        
        # Sensor Data Frame
        sensor_frame = ttk.LabelFrame(self.root, text="Current Sensor Data", padding=10)
        sensor_frame.pack(fill="x", padx=10, pady=5)
        
        tk.Label(sensor_frame, text="Temperature:").grid(row=0, column=0, sticky="w")
        temp_label = tk.Label(sensor_frame, textvariable=self.current_temp, 
                             font=("Arial", 14, "bold"), fg="red")
        temp_label.grid(row=0, column=1, sticky="w")
        tk.Label(sensor_frame, text="°C").grid(row=0, column=2, sticky="w")
        
        tk.Label(sensor_frame, text="Humidity:").grid(row=1, column=0, sticky="w")
        humid_label = tk.Label(sensor_frame, textvariable=self.current_humidity,
                              font=("Arial", 12, "bold"), fg="blue")
        humid_label.grid(row=1, column=1, sticky="w")
        tk.Label(sensor_frame, text="%").grid(row=1, column=2, sticky="w")
        
        tk.Label(sensor_frame, text="Motor Status:").grid(row=2, column=0, sticky="w")
        status_label = tk.Label(sensor_frame, textvariable=self.motor_status,
                               font=("Arial", 12, "bold"), fg="green")
        status_label.grid(row=2, column=1, sticky="w")
        
        # Threshold Control Frame
        threshold_frame = ttk.LabelFrame(self.root, text="Temperature Threshold Settings", padding=10)
        threshold_frame.pack(fill="x", padx=10, pady=5)
        
        tk.Label(threshold_frame, text="Set Temperature Threshold (°C):").pack(anchor="w", pady=5)
        
        # Threshold input
        input_frame = tk.Frame(threshold_frame)
        input_frame.pack(fill="x", pady=5)
        
        tk.Label(input_frame, text="Temperature:").pack(side="left")
        threshold_spin = tk.Spinbox(input_frame, from_=15, to=50, textvariable=self.threshold_temp,
                                   width=10, font=("Arial", 12))
        threshold_spin.pack(side="left", padx=5)
        tk.Label(input_frame, text="°C").pack(side="left")
        
        def set_threshold():
            try:
                temp = self.threshold_temp.get()
                self.send_command("set_threshold", {"temperature": temp})
                messagebox.showinfo("Success", f"Threshold set to {temp}°C")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to set threshold: {str(e)}")
        
        tk.Button(threshold_frame, text="Set Threshold", command=set_threshold,
                 bg="blue", fg="white", font=("Arial", 10, "bold")).pack(pady=10)
        
        # Manual Control Frame
        control_frame = ttk.LabelFrame(self.root, text="Manual Motor Control", padding=10)
        control_frame.pack(fill="x", padx=10, pady=5)
        
        button_frame = tk.Frame(control_frame)
        button_frame.pack(fill="x")
        
        def motor_on():
            try:
                self.send_command("motor_control", {"action": "on"})
                messagebox.showinfo("Success", "Motor turned ON")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to turn on motor: {str(e)}")
        
        def motor_off():
            try:
                self.send_command("motor_control", {"action": "off"})
                messagebox.showinfo("Success", "Motor turned OFF")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to turn off motor: {str(e)}")
        
        tk.Button(button_frame, text="🔴 Motor ON", command=motor_on,
                 bg="green", fg="white", font=("Arial", 10, "bold"), padx=20).pack(side="left", padx=5)
        tk.Button(button_frame, text="⚫ Motor OFF", command=motor_off,
                 bg="red", fg="white", font=("Arial", 10, "bold"), padx=20).pack(side="left", padx=5)
        
        # Status Frame
        status_frame = tk.Frame(self.root)
        status_frame.pack(fill="x", padx=10, pady=10)
        
        self.status_label = tk.Label(status_frame, text="Waiting for connection...", 
                                    fg="orange", font=("Arial", 9))
        self.status_label.pack(anchor="w")
        
        # Auto-refresh checkbox
        self.auto_refresh = tk.BooleanVar(value=True)
        tk.Checkbutton(status_frame, text="Auto-refresh data (every 2s)", 
                      variable=self.auto_refresh).pack(anchor="w")
    
    def send_command(self, command, data=None):
        """Send command to ESP8266"""
        try:
            url = f"{self.base_url}/{command}"
            response = requests.post(url, json=data, timeout=5)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            self.status_label.config(text=f"❌ Connection Error: {str(e)}", fg="red")
            raise
    
    def update_data(self):
        """Update sensor data from ESP8266"""
        def fetch_data():
            try:
                response = self.send_command("get_data")
                self.current_temp.set(response.get("temperature", 0.0))
                self.current_humidity.set(response.get("humidity", 0.0))
                motor = response.get("motor_status", "OFF")
                self.motor_status.set(motor)
                self.status_label.config(
                    text=f"✅ Connected | Updated: {datetime.now().strftime('%H:%M:%S')}", 
                    fg="green"
                )
            except Exception as e:
                self.status_label.config(text=f"❌ Error: {str(e)}", fg="red")
        
        if self.auto_refresh.get():
            thread = threading.Thread(target=fetch_data, daemon=True)
            thread.start()
        
        self.root.after(2000, self.update_data)

if __name__ == "__main__":
    root = tk.Tk()
    app = TemperatureControllerApp(root)
    root.mainloop()
