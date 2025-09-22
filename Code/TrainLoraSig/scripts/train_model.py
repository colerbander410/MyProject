import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.metrics import confusion_matrix, classification_report
import tensorflow as tf

# 1. Read data
input_file = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/labels/labeled_data.xlsx'  
df = pd.read_excel(input_file, sheet_name='labeled_data')

print("Columns in file:", df.columns)

# 2. Select features and target
features = ["rssi", "snr"]
target = "quality_quantile"

X = df[features].values
y = df[target].values

# 3. Encode labels to numbers
encoder = LabelEncoder()
y_encoded = encoder.fit_transform(y)

# 4. Split train/test
X_train, X_test, y_train, y_test = train_test_split(
    X, y_encoded, test_size=0.2, random_state=42, stratify=y_encoded
)

# 5. Standardize features
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

# 6. Build TinyML model (2-3 Dense layers, compact)
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(X_train.shape[1],)),
    tf.keras.layers.Dense(16, activation="relu"),   # small hidden layer
    tf.keras.layers.Dense(8, activation="relu"),
    tf.keras.layers.Dense(len(np.unique(y_encoded)), activation="softmax")  # output
])

model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"]
)

# 7. Train
history = model.fit(
    X_train, y_train,
    validation_data=(X_test, y_test),
    epochs=30,
    batch_size=16,
    verbose=1
)

# 8. Evaluate
y_pred = np.argmax(model.predict(X_test), axis=1)
print("Confusion Matrix:\n", confusion_matrix(y_test, y_pred))
print("Classification Report:\n", classification_report(y_test, y_pred, target_names=encoder.classes_))

# 9. Export model to TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open("lora_quality_model.tflite", "wb") as f:
    f.write(tflite_model)

print("✅ Model has been saved as lora_quality_model.tflite")

# 10. Save scaler and encoder for microcontroller use (to be converted to constants later)
import joblib
joblib.dump(scaler, "scaler.pkl")
joblib.dump(encoder, "encoder.pkl")
