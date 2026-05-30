import socket

ARDUINO_IP = "192.168.1.100"  # Cambiar por la IP que muestre el monitor serial
ARDUINO_PORT = 5000
TIMEOUT_S = 5


def check_connection(ip: str, port: int) -> bool:
    try:
        with socket.create_connection((ip, port), timeout=TIMEOUT_S) as s:
            s.sendall(b"PING\n")
            response = s.recv(1024).decode("utf-8").strip()
            print(f"Respuesta del Arduino: {response}")
            return response == "PONG"
    except Exception as e:
        print(f"Error de conexion: {e}")
        return False


if __name__ == "__main__":
    ok = check_connection(ARDUINO_IP, ARDUINO_PORT)
    if ok:
        print("Conexion OK entre app y Arduino")
    else:
        print("Fallo de conexion o respuesta invalida")
