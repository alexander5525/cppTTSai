# C++ Text-to-Speech AI 🗣️🎶

## 📜 Description
This project was an early 2022 attempt at creating a sophisticated text-to-speech (TTS) system before platforms like ElevenLabs became widely available. It was a challenging and ambitious endeavor that combined neural networks, audio encoding, and phonetic transcription.

## 💡 The Idea
The project was designed to convert short text inputs into corresponding audio outputs using a combination of neural networks and compact audio representations. The workflow included:

1. **Autoencoder**:  
   - Encode audio files (up to 16 KB, each containing a spoken word or sound) into a highly compact representation of \( 2^8 \)-\( 2^9 \) bits.
   - **Key Mistake**: I initially attempted to encode the binary data of `.ogg` files directly, which proved unsuitable.

2. **Hand-Transcription**:  
   - Each of the 3,122 audio files was manually transcribed into phonetic symbols corresponding to individual sounds, mapped in a `Text Legend.txt` file.

3. **Second Model**:  
   - A network designed to transform short text inputs (up to 15 characters) into the compact audio representation created by the autoencoder.

4. **Final Pipeline**:  
   - Stitch the autoencoder and text-to-compact-audio models into a complete TTS system:
     - Input: Text (15 characters).
     - Intermediate Layer: Compact audio representation (\( 2^8 \)-\( 2^9 \) bits).
     - Output: Full audio file.

## 🚀 Accomplishments
- Successfully implemented the **autoencoder neural network** in C++.
- Explored innovative ways to compact audio data, though encoding raw `.ogg` files proved to be a critical limitation.

## 🤔 Challenges & Lessons Learned
- **Challenges**:
  - Encoding `.ogg` files directly instead of spectrograms was an inefficient approach for this problem.
  - Designing compact and effective representations of audio data was more complex than anticipated.
- **Lessons Learned**:
  - The intricacies of neural networks, particularly the mechanics of backpropagation.
  - The importance of choosing appropriate input representations for machine learning tasks.

## 🛠️ Improvements for the Future
If I were to revisit this project, I would:
1. Use **spectrograms** as the input for the audio encoding process instead of raw `.ogg` files.
2. Build the model using **Convolutional Neural Networks (CNNs)**, leveraging libraries like TensorFlow or PyTorch for a more streamlined development process.
3. Incorporate pre-trained models or frameworks to improve performance and reduce development overhead.

## 📂 Important Files
Here are the key files in this repository:
- `TTS_AI.cpp`: Contains the main implementation of the neural network (autoencoder).
- `TTS_AI_recreator.cpp`: Recreates the audio file from the network.
- `TTS_Byte.cpp`: Generates a list of byte-level differences between `.ogg` files to compact data further.
- Other files: Legacy code for auxiliary tasks (may require additional context).

⚠️ **Note**: The dataset used for training is not included in this repository to respect the privacy of the individual whose voice was recorded.
**Also**: The current state is of when it was last used i.e. some important methods may be commented out.

⚠️ **Disclaimer**: This code is published for educational and portfolio purposes. If you'd like to use or reference any part of it, please provide proper attribution or contact me for permission.

## 🌟 Key Takeaways
While this project didn't achieve its ultimate goal, it served as an excellent learning experience in neural network design, debugging, and understanding the nuances of machine learning workflows.

