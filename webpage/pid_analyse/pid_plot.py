import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load the CSV file
file_path = "/home/sai/Downloads/2025_03_04_17_43_00_988_.csv"  # Change to your actual file path
df = pd.read_csv(file_path, delimiter=None, engine='python')  # Auto-detect delimiter

print("started")

print("Column Names:", df.columns.tolist())

# Create a counter column instead of using timestamp
df["counter"] = range(len(df))

# --- 1. Plot Roll, Pitch, Yaw, and Altitude ---
plt.figure(figsize=(12, 6))

plt.subplot(2, 2, 1)
plt.plot(df["counter"], df["rollData"], label="Roll", color="b")
plt.xlabel("Samples")
plt.ylabel("Roll (degrees)")
plt.title("Roll over Samples")
plt.legend()

plt.subplot(2, 2, 2)
plt.plot(df["counter"], df["pitchData"], label="Pitch", color="r")
plt.xlabel("Samples")
plt.ylabel("Pitch (degrees)")
plt.title("Pitch over Samples")
plt.legend()

plt.subplot(2, 2, 3)
plt.plot(df["counter"], df["yawData"], label="Yaw", color="g")
plt.xlabel("Samples")
plt.ylabel("Yaw (degrees)")
plt.title("Yaw over Samples")
plt.legend()

plt.subplot(2, 2, 4)
plt.plot(df["counter"], df["altitudeData"], label="Altitude", color="purple")
plt.xlabel("Samples")
plt.ylabel("Altitude (m)")
plt.title("Altitude over Samples")
plt.legend()

plt.tight_layout()
plt.show()

# --- 2. Plot PID Errors vs. Outputs ---
plt.figure(figsize=(12, 6))

for i in range(1, 5):
    plt.subplot(2, 2, i)
    plt.plot(df["counter"], df[f"error{i}Data"], label=f"Error {i}", linestyle="dashed")
    plt.plot(df["counter"], df[f"pidOut{i}Data"], label=f"PID Output {i}")
    plt.xlabel("Samples")
    plt.ylabel("Value")
    plt.title(f"Error {i} vs. PID Output {i}")
    plt.legend()

plt.tight_layout()
plt.show()

# --- 3. Plot Motor Speeds Over Samples ---
plt.figure(figsize=(12, 6))

for i in range(1, 5):
    plt.plot(df["counter"], df[f"motorSpeed{i}Data"], label=f"Motor {i}")

plt.xlabel("Samples")
plt.ylabel("Motor Speed (PWM or RPM)")
plt.title("Motor Speeds Over Samples")
plt.legend()
plt.grid()

plt.show()

# --- 4. Correlation Analysis (Heatmap) ---
plt.figure(figsize=(10, 6))
sns.heatmap(df.corr(), annot=True, cmap="coolwarm", fmt=".2f", linewidths=0.5)
plt.title("Correlation Between Drone Parameters")
plt.show()

