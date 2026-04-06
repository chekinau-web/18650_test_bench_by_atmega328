% Очистка рабочего пространства
clear; clc; close all;

%% 1. ЗАГРУЗКА ДАННЫХ ИЗ ФАЙЛА
% Имя файла (укажите свой путь, если файл не в рабочей папке)
filename = '..\01_arduino_code\Blink_display_5\ee6.eep';

% Читаем все числа из файла (разделители: запятые, пробелы, переводы строк)
data_bytes = load(filename);  % если файл содержит только числа через пробел/запятую

% Альтернативный вариант (если load не справляется с запятыми):
% str = fileread(filename);
% numbers = regexp(str, '[\d]+', 'match');
% data_bytes = str2double(numbers);

fprintf('Загружено байт: %d\n', length(data_bytes));

%% 2. ПРЕОБРАЗОВАНИЕ В 16-БИТНЫЕ ЗНАЧЕНИЯ (LITTLE-ENDIAN)
% Убедимся, что количество байт чётное
if mod(length(data_bytes), 2) ~= 0
    warning('Количество байт нечётное! Последний байт будет отброшен.');
    data_bytes = data_bytes(1:end-1);
end

% Преобразуем пары байт в uint16 (младший байт первый)
numSamples = length(data_bytes) / 2;
adcValues = zeros(1, numSamples, 'uint16');

for i = 1:numSamples
    lowByte = data_bytes(2*i - 1);
    highByte = data_bytes(2*i);
    adcValues(i) = lowByte + bitshift(highByte, 8);
end

fprintf('Получено значений АЦП: %d\n', length(adcValues));

%% 3. (ОПЦИОНАЛЬНО) ПЕРЕСЧЁТ В НАПРЯЖЕНИЕ
Vref = 5.0;  % опорное напряжение (REF195)
voltages = double(adcValues) * (Vref / 1023.0);

%% 3.5. ГЕНЕРАЦИЯ ОЖИДАЕМЫХ ЗНАЧЕНИЙ НАПРЯЖЕНИЯ (ЗАДАННЫХ ЧЕРЕЗ PWM)
% Параметры изменения PWM (из Arduino-скетча)
PWM_STEP = 10;          % шаг изменения
PWM_MAX = 168;          % максимальное значение (соответствует 3.3 В)
Vref = 5.0;             % опорное напряжение (REF195)
samplesPerStep = 50;    % 5 секунд * 10 измерений/сек = 50 отсчётов на шаг

% Генерируем массив ожидаемых значений pwmValue для каждого измерения
expectedPwm = zeros(1, numSamples);
currentPwm = 0;
for i = 1:numSamples
    expectedPwm(i) = currentPwm;
    % Меняем pwmValue каждые samplesPerStep отсчётов
    if mod(i, samplesPerStep) == 0
        currentPwm = currentPwm + PWM_STEP;
        if currentPwm > PWM_MAX
            currentPwm = 0;
        end
    end
end

% Ожидаемое напряжение (без учёта фильтра)
expectedVoltages = expectedPwm * (Vref / 255);   % так как analogWrite принимает 0..255

fprintf('Сгенерировано ожидаемых значений: %d\n', length(expectedVoltages));

%% 4. ПОСТРОЕНИЕ ГРАФИКОВ
figure('Name', 'Анализ EEPROM', 'Position', [100 100 1400 500]);

% График сырых значений АЦП
subplot(1,2,1);
plot(adcValues, 'b-', 'LineWidth', 1);
hold on;
expectedADC = expectedPwm * (1023 / 255);   % ожидаемые значения АЦП (0-1023)
plot(expectedADC, 'r--', 'LineWidth', 1);
hold off;
title('Значения АЦП (0-1023)');
xlabel('Номер измерения (10 измерений/сек)');
ylabel('ADC value');
legend('Измеренное', 'Ожидаемое (заданное через PWM)', 'Location', 'best');
grid on;
xlim([1 length(adcValues)]);

% График напряжения
subplot(1,2,2);
plot(voltages, 'b-', 'LineWidth', 1);
hold on;
plot(expectedVoltages, 'r--', 'LineWidth', 1);
hold off;
title('Напряжение, В');
xlabel('Номер измерения (10 измерений/сек)');
ylabel('Напряжение, В');
legend('Измеренное (после фильтра)', 'Ожидаемое (заданное)', 'Location', 'best');
grid on;
xlim([1 length(voltages)]);

%% 5. (ДОПОЛНИТЕЛЬНО) СТАТИСТИКА
fprintf('\n--- Статистика ---\n');
fprintf('Минимальное ADC: %d\n', min(adcValues));
fprintf('Максимальное ADC: %d\n', max(adcValues));
fprintf('Среднее ADC: %.2f\n', mean(adcValues));
fprintf('Минимальное напряжение: %.3f В\n', min(voltages));
fprintf('Максимальное напряжение: %.3f В\n', max(voltages));
fprintf('Среднее напряжение: %.3f В\n', mean(voltages));

%% 6. (ОПЦИОНАЛЬНО) СОХРАНЕНИЕ РЕЗУЛЬТАТОВ В ФАЙЛ
% Сохранить adcValues и voltages в MAT-файл
save('eeprom_analysis.mat', 'adcValues', 'voltages');
fprintf('\nРезультаты сохранены в eeprom_analysis.mat\n');