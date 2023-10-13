import subprocess
import threading


def start_client(ip_address, port, buffer_size, filename):
    # Construct the command to start the client
    command = f"./sendFile {filename} {ip_address}:{port} {buffer_size}"

    # Run the command using subprocess
    subprocess.run(command, shell=True)


def main():
    ip_address = input("Enter the IP address: ")
    port = input("Enter the port number: ")
    buffer_size = input("Enter the buffer size: ")

    filename_options = ["wonderland.txt",
                        "test1.txt", "test2.txt", "test3.txt"]

    # Create threads for each client
    threads = []
    for filename in filename_options:
        thread = threading.Thread(target=start_client, args=(
            ip_address, port, buffer_size, filename))
        threads.append(thread)
        thread.start()

    # Wait for all threads to finish
    for thread in threads:
        thread.join()


if __name__ == "__main__":
    main()
