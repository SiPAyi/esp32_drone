import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt

def compute_overshoot(setpoint, actual):
    max_value = np.max(actual)
    overshoot = ((max_value - setpoint) / setpoint) * 100 if setpoint != 0 else 0
    return overshoot

def analyze_pid(csv_file, window=1000):
    """ Reads last 'window' points from CSV and analyzes Roll PID performance """
    df = pd.read_csv(csv_file)
    df = df.tail(window)  # Get latest data
    
    roll_setpoint = df["Roll_Setpoint"]
    roll_actual = df["Roll_Actual"]
    
    # Compute errors
    roll_error = np.mean(roll_setpoint - roll_actual)
    overshoot = compute_overshoot(roll_setpoint.iloc[-1], roll_actual)
    
    # Suggest PID tuning based on error patterns
    pid_suggestion = ""
    if abs(roll_error) > 5:
        pid_suggestion += "Increase Kp to reduce steady-state error.\n"
    if overshoot > 10:
        pid_suggestion += "Reduce Kp and increase Kd to control overshoot.\n"
    if overshoot < 2 and roll_error < 1:
        pid_suggestion += "PID is well-tuned!\n"
    
    print("\n--- PID Analysis ---")
    print(f"Roll Steady-State Error: {roll_error:.2f} degrees")
    print(f"Roll Overshoot: {overshoot:.2f}%")
    print(f"Suggested Tuning: \n{pid_suggestion}")
    
    # Plot response
    plt.figure(figsize=(8, 4))
    plt.plot(df["Time"], roll_setpoint, label="Roll Setpoint", linestyle="dashed")
    plt.plot(df["Time"], roll_actual, label="Roll Actual")
    plt.xlabel("Time (s)")
    plt.ylabel("Roll (degrees)")
    plt.legend()
    plt.title("Roll Response Analysis")
    plt.show()

# Run analysis in real-time
csv_file = "drone_data.csv"  # Update with actual file
while True:
    analyze_pid(csv_file, window=1000)
    time.sleep(2)  # Run analysis every 2 seconds
