# 🌈 CPU Scheduling Simulator  

> A colorful, animated GUI simulator for CPU scheduling algorithms built in **C (GTK3 + Cairo)**  

<img width="1919" height="1046" alt="image" src="https://github.com/user-attachments/assets/f9ec5556-f127-461a-b75e-a872e5347a28" />


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

| Main Window | Gantt Chart | About Dialog |
|--------------|-------------|---------------|
|<img width="1919" height="1046" alt="image" src="https://github.com/user-attachments/assets/d9334a4d-6a0b-4f5b-b5b1-55f0a164fddb" />
 | <img width="1916" height="281" alt="image" src="https://github.com/user-attachments/assets/d2fef74e-a4b6-47a4-825a-1dc105e4d2b3" />
| <img width="522" height="339" alt="image" src="https://github.com/user-attachments/assets/6ce87231-8ecc-4091-82c6-5ea1f3ac2108" />
 |

---

## 💻 How to Run  

### 🧱 Option 1: Use the Prebuilt AppImage  
Download the latest release from [**Releases**](https://github.com/ParthBorax/CPU-Scheduling-Application/releases).  

Then run:
```bash
chmod +x CPU_Scheduling_Simulator-x86_64.AppImage
./CPU_Scheduling_Simulator-x86_64.AppImage
