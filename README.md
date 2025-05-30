# Point-to-Point-Protocol
# PPP Protocol Implementation with QP/C

This project implements a simplified version of the **PPP (Point-to-Point Protocol)** in C using the **QP/C (Quantum Platform)** real-time framework and **UDP sockets** to simulate communication between two endpoints. The protocol is modeled using events and hierarchical state machines with HDLC-style framing, according to the PPP specification.

---

## 💡 Project Goals

Simulate a PPP connection between two endpoints, including:

- `Configure-Request` and `Configure-Ack` negotiation
- Message exchange through `Payload` packets
- Acknowledgment using `Payload-Ack`
- Flow control using state machines and signals
- User interaction via console to accept connections and send messages

---

## 📁 Project Structure

| File             | Description |
|------------------|-------------|
| `main.c`         | Application entry point. Initializes QP/C and starts active objects (AOs). |
| `ppp.c/.h`       | PPP logic and hierarchical state machine (e.g., Starting, Opened). |
| `bsp.c/.h`       | Board Support Package: handles UDP socket communication, HDLC framing, CRC, and user I/O. |
| `sinais.h`       | Defines custom signals and events for the PPP state machine. |
| `pse19-ppp.pdf`  | Reference document or protocol design notes. |

---

## 🛠 Requirements

- [QP/C Framework](https://www.state-machine.com/qpc/)
- ANSI C compiler (GCC, MSVC, etc.)
- Windows with Winsock support

---

## ✅ Features Implemented

- ✅ Hierarchical state machine using QP/C
- ✅ HDLC-style PPP framing over UDP
- ✅ LCP configuration and negotiation
- ✅ Interactive message transmission
- ✅ User-controlled session management
- ✅ Dynamic memory allocation for variable-length payloads

---

## 📌 Notes

- This is a **didactic simulation** of PPP, not a full stack implementation.
- Authentication (PAP/CHAP), compression, and real-time error handling are **not implemented**.
- Use this as a reference or starting point for learning event-driven state machines and communication protocols.

---

## 📄 License

This software is distributed under the **GNU General Public License v3 (GPLv3)**  
Alternatively, it may be used under a **commercial license from Quantum Leaps**.

See the license header in each source file for details.

