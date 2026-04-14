#!/bin/bash

# сначала собираем программу
make clean
make
if [ $? -ne 0 ]; then
    echo "Ошибка сборки программы" > result.txt
    exit 1
fi

echo "Отчет о тестировании sparse файлов" > result.txt
echo "==================================" >> result.txt
echo "" >> result.txt

# создаем файл A
echo "Создаем тестовый файл A" >> result.txt

dd if=/dev/zero of=A bs=1 count=4194305 2> /dev/null
if [ $? -ne 0 ]; then
    echo "Не удалось создать файл A" >> result.txt
    exit 1
fi

# записываем единички в позиции 0, 10000 и в конец файла
echo -n -e '\x01' | dd of=A bs=1 seek=0 conv=notrunc 2> /dev/null
echo -n -e '\x01' | dd of=A bs=1 seek=10000 conv=notrunc 2> /dev/null
echo -n -e '\x01' | dd of=A bs=1 seek=4194304 conv=notrunc 2> /dev/null

echo "Файл A создан, размер: " >> result.txt
stat -c "%s" A >> result.txt
echo " байт" >> result.txt

# копируем A в B 
echo "" >> result.txt
echo "Тест: ./sparse A B" >> result.txt
./sparse A B

# сжимаем A и B
echo "Сжимаем файлы" >> result.txt
gzip -c A > A.gz
gzip -c B > B.gz
echo "Созданы файлы A.gz и B.gz" >> result.txt

# распаковываем B.gz и сохраняем в C
echo "" >> result.txt
echo "Тест: gzip -cd B.gz | ./sparse C" >> result.txt
gzip -cd B.gz | ./sparse C

# копируем A в D с размером блока 100 байт
echo "" >> result.txt
echo "Тест: ./sparse -b 100 A D" >> result.txt
./sparse -b 100 A D

# выводим размеры файлов
echo "" >> result.txt
echo "Размеры файлов:" >> result.txt
echo "--------------------------------" >> result.txt
echo "Файл   Логический размер   Физический размер" >> result.txt

for f in A A.gz B B.gz C D; do
    if [ -f $f ]; then
        logical=`stat -c %s $f`
        blocks=`stat -c %b $f`
        block_size=`stat -c %B $f`
        physical=`expr $blocks \* $block_size`
        echo "$f      $logical           $physical" >> result.txt
    fi
done

echo "" >> result.txt
echo "Тестирование завершено." >> result.txt

exit 0