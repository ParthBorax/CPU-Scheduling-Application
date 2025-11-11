# 🌈 CPU Scheduling Simulator  

> A colorful, animated GUI simulator for CPU scheduling algorithms built in **C (GTK3 + Cairo)**  

<p align="center">
  <img src="https://github.com/user-attachments/assets/f9ec5556-f127-461a-b75e-a872e5347a28" width="90%" alt="CPU Scheduling Simulator main window" />
</p>

---

## 🚀 Overview  

**CPU Scheduling Simulator** is a desktop application developed in **C** using **GTK3** and **Cairo** graphics.  
It visually demonstrates key CPU scheduling algorithms used in **Operating Systems Fundamentals (OSF)**  
through an interactive interface and animated Gantt charts.  

This project was created as part of my **B.Tech (Electronics & Telecommunication)** coursework,  
and submitted to **Dr. Nilima Zade**, Department of Computer Science.  

---

## 🧩 Features  

✨ **Visual Simulation:** Animated, color-coded Gantt chart  
🎨 **Modern UI:** Styled using GTK CSS for a vibrant aesthetic  
⚙️ **Multiple Algorithms Supported:**  
- FCFS (First Come, First Served)  
- SJF (Shortest Job First – Non-preemptive)  
- Priority Scheduling (Non-preemptive)  
- Round Robin  
- SJF (Preemptive / SRTF)  
- Priority (Preemptive)  

📊 **Metrics Displayed:**  
- Waiting Time  
- Turnaround Time  
- Average Waiting and Turnaround  

🧠 **Extras:**  
- “About” dialog with author details  
- Built-in animation and gradient transitions  
- Works as a **portable AppImage** — no install needed  

---

## 🖼️ Screenshots  

<div align="center">

### 🪟 Main Window  
<img src="https://github.com/user-attachments/assets/d9334a4d-6a0b-4f5b-b5b1-55f0a164fddb" width="85%" alt="Main Window" />

---

### 📊 Gantt Chart Visualization  
<img src="https://github.com/user-attachments/assets/d2fef74e-a4b6-47a4-825a-1dc105e4d2b3" width="85%" alt="Gantt Chart" />

---

### ℹ️ About Dialog  
<img src="https://github.com/user-attachments/assets/6ce87231-8ecc-4091-82c6-5ea1f3ac2108" width="45%" alt="About Dialog" />

</div>

---

## 💻 How to Run  

### 🧱 Option 1: Use the Prebuilt AppImage  
Download the latest release from [**Releases**](https://github.com/ParthBorax/CPU-Scheduling-Application/releases).  

Then run:
```bash
chmod +x CPU_Scheduling_Simulator-x86_64.AppImage
./CPU_Scheduling_Simulator-x86_64.AppImage
