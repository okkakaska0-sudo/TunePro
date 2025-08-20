# MarsiAutoTune - Инструкции по загрузке на macOS

## 📥 Загрузка проекта из Replit

1. **Скачайте весь проект:**
   - Нажмите на три точки (...) в файловом менеджере Replit
   - Выберите "Download as zip"
   - Сохраните архив на ваш Mac

2. **Распакуйте и подготовьте:**
   ```bash
   cd ~/Downloads
   unzip replit-export.zip
   cd MarsiAutoTune  # или имя распакованной папки
   ```

3. **Проверьте структуру проекта:**
   ```bash
   ls -la libs/
   # Должны быть папки: crepe, eigen, fftw, rubberband, tensorflow_lite
   ```

4. **Запустите сборку:**
   ```bash
   chmod +x build_simple.sh
   ./build_simple.sh
   ```

## 🔧 Требования для macOS

- macOS 10.13+ (протестировано на 14.7.5)
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake`

## 📦 Что будет установлено

- **VST3:** `~/Library/Audio/Plug-Ins/VST3/MarsiAutoTune.vst3`
- **AU:** `~/Library/Audio/Plug-Ins/Components/MarsiAutoTune.component`
- **Standalone:** `/Applications/MarsiAutoTune.app`

## 🎵 Совместимые DAW

- Logic Pro X
- Pro Tools  
- Ableton Live
- FL Studio
- Reaper
- Studio One
- Cubase

## 📊 Размеры

- **Исходники:** ~3.5MB (включая все библиотеки)
- **Собранный плагин:** ~50MB
- **Время сборки:** 3-5 минут

Проект полностью автономен - интернет нужен только для первоначальной загрузки JUCE на macOS.
