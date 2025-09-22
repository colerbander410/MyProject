import pandas as pd
import numpy as np

# --- 1. Read the Excel file ---
input_file = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/processed/cleaned_lora_with_trace.xlsx'
df = pd.read_excel(input_file, sheet_name='cleaned_robust')

# --- 3. Compute quantiles per (node_id, gateway) in vectorized way ---
group_keys = ['node_id', 'gateway']
df['q20_rssi'] = df.groupby(group_keys)['rssi'].transform(lambda x: x.quantile(0.2))
df['q80_rssi'] = df.groupby(group_keys)['rssi'].transform(lambda x: x.quantile(0.8))
df['q20_snr'] = df.groupby(group_keys)['snr'].transform(lambda x: x.quantile(0.2))
df['q80_snr'] = df.groupby(group_keys)['snr'].transform(lambda x: x.quantile(0.8))

# --- 4. Assign quality_quantile labels ---
conditions = [
    (df['rssi'] >= df['q80_rssi']) & (df['snr'] >= df['q80_snr']),
    (df['rssi'] <= df['q20_rssi']) | (df['snr'] <= df['q20_snr'])
]
choices = ['good', 'poor']
df['quality_quantile'] = np.select(conditions, choices, default='fair')

# --- 5. Select best gateway per timestamp/node in vectorized way ---
# Sort by timestamp, node_id ascending, SNR descending, RSSI descending
df.sort_values(['timestamp', 'node_id', 'snr', 'rssi'], ascending=[True, True, False, False], inplace=True)

# Mark first row per (timestamp, node_id) as best gateway
df['is_best_gateway'] = ~df.duplicated(subset=['timestamp', 'node_id'], keep='first')

# --- 6. Optional: position_known if latitude/longitude exist ---
if 'latitude' in df.columns and 'longitude' in df.columns:
    df['position_known'] = df[['latitude', 'longitude']].notna().all(axis=1)

# --- 7. Save full labeled dataframe to Excel ---
output_full = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/labels/labeled_data.xlsx'
df.to_excel(output_full, sheet_name='labeled_data', index=False)
print(f"Full labeled dataset saved to {output_full}")

# --- 8. Save best gateway table to CSV ---
best_gateway_df = df[df['is_best_gateway']][['timestamp', 'node_id', 'gateway']].copy()
best_gateway_df.rename(columns={'gateway':'best_gateway'}, inplace=True)

output_best = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/labels/best_gateway_per_node_time.csv'
best_gateway_df.to_csv(output_best, index=False)
print(f"Best gateway table saved to {output_best}")

print("All steps completed successfully.")
