# VTL Project

## Запуск приложения

1. Создайте директорию для сборки:
```bash
mkdir build
```

2. Сконфигурируйте проект:
```bash
cmake -B build
```

3. Соберите проект:
```bash
cmake --build build
```

4. Запустите приложение:
```bash
./app/VTL
```

## Тестирование обработки изображений

Для тестирования обработки изображений:

1. Создайте директорию для тестовых изображений:
```bash
mkdir -p test_images
```

2. Скопируйте тестовое изображение в директорию `test_images`:
```bash
cp doc/Lenna.png test_images/
```

3. Скопируйте тестовый main.c:
```bash
cp doc/main.c .
```

4. Пересоберите проект:
```bash
cmake --build build
```

5. Запустите тест:
```bash
./app/VTL
```

Результат обработки будет сохранен в файл `test_images/Lenna_processed.png`.

## Требования

- CMake 3.25.0 или выше
- FFmpeg с поддержкой:
  - libavcodec
  - libavformat
  - libavfilter
  - libswscale
  - libavutil
- Компилятор C с поддержкой C11

