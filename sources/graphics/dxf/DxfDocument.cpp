#include "graphics/dxf/DxfDocument.h"

#include <charconv>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>


namespace
{
    /*
     * Форматирование числа для DXF без искусственного округления.
     *
     * std::to_chars(..., chars_format::fixed) использует кратчайшее
     * представление, которое однозначно восстанавливает исходный double.
     * Поэтому координата 2209858.5177 записывается именно как
     * 2209858.5177, а не обрезается до 6 значащих цифр и не получает
     * длинный хвост двоичного представления double.
     */
    std::string formatDxfNumber(double value)
    {
        char buffer[64]{};

        const auto result =
            std::to_chars(
                buffer,
                buffer + sizeof(buffer),
                value,
                std::chars_format::fixed);

        if (result.ec == std::errc{})
            return std::string(buffer, result.ptr);

        std::ostringstream fallback;
        fallback << std::setprecision(17) << value;
        return fallback.str();
    }

    /*
     * ------------------------------------------------------------
     * Преобразование UTF-8 -> DXF Unicode escape
     * ------------------------------------------------------------
     *
     * AutoCAD поддерживает Unicode в DXF через последовательности:
     *
     *     \U+XXXX
     *
     * Например:
     *
     *     Песок
     *
     * превращается в:
     *
     *     \U+041F\U+0435\U+0441\U+043E\U+043A
     *
     * Это позволяет корректно передавать кириллицу независимо
     * от кодировки исходного файла C++.
     */

    std::string encodeUtf8ToDxfUnicode(
        const std::string& text)
    {
        std::string result;

        result.reserve(
            text.size() * 2);

        std::size_t i = 0;

        while (i < text.size())
        {
            const unsigned char c =
                static_cast<unsigned char>(text[i]);

            /*
             * ----------------------------------------------------
             * Обычный ASCII
             * ----------------------------------------------------
             *
             * Символы от 0x01 до 0x7F оставляем как есть.
             */

            if (c < 0x80)
            {
                result +=
                    static_cast<char>(c);

                ++i;
                continue;
            }

            /*
             * ----------------------------------------------------
             * Декодируем UTF-8 в Unicode code point.
             * ----------------------------------------------------
             */

            std::uint32_t codePoint = 0;

            if ((c & 0xE0) == 0xC0)
            {
                /*
                 * 2-byte UTF-8
                 */

                if (i + 1 >= text.size())
                {
                    result += '?';
                    break;
                }

                const unsigned char c1 =
                    static_cast<unsigned char>(
                        text[i + 1]);

                if ((c1 & 0xC0) != 0x80)
                {
                    result += '?';
                    ++i;
                    continue;
                }

                codePoint =
                    ((c & 0x1F) << 6) |
                    (c1 & 0x3F);

                i += 2;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                /*
                 * 3-byte UTF-8
                 */

                if (i + 2 >= text.size())
                {
                    result += '?';
                    break;
                }

                const unsigned char c1 =
                    static_cast<unsigned char>(
                        text[i + 1]);

                const unsigned char c2 =
                    static_cast<unsigned char>(
                        text[i + 2]);

                if ((c1 & 0xC0) != 0x80 ||
                    (c2 & 0xC0) != 0x80)
                {
                    result += '?';
                    ++i;
                    continue;
                }

                codePoint =
                    ((c & 0x0F) << 12) |
                    ((c1 & 0x3F) << 6) |
                    (c2 & 0x3F);

                i += 3;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                /*
                 * 4-byte UTF-8
                 */

                if (i + 3 >= text.size())
                {
                    result += '?';
                    break;
                }

                const unsigned char c1 =
                    static_cast<unsigned char>(
                        text[i + 1]);

                const unsigned char c2 =
                    static_cast<unsigned char>(
                        text[i + 2]);

                const unsigned char c3 =
                    static_cast<unsigned char>(
                        text[i + 3]);

                if ((c1 & 0xC0) != 0x80 ||
                    (c2 & 0xC0) != 0x80 ||
                    (c3 & 0xC0) != 0x80)
                {
                    result += '?';
                    ++i;
                    continue;
                }

                codePoint =
                    ((c & 0x07) << 18) |
                    ((c1 & 0x3F) << 12) |
                    ((c2 & 0x3F) << 6) |
                    (c3 & 0x3F);

                i += 4;
            }
            else
            {
                /*
                 * Некорректный UTF-8.
                 */

                result += '?';
                ++i;
                continue;
            }

            /*
             * ----------------------------------------------------
             * DXF Unicode escape
             * ----------------------------------------------------
             *
             * Для символов BMP используем:
             *
             *     \U+XXXX
             *
             * Для символов выше U+FFFF используем две
             * UTF-16 surrogate-последовательности.
             */

            if (codePoint <= 0xFFFF)
            {
                std::ostringstream unicode;

                unicode
                    << "\\U+"
                    << std::uppercase
                    << std::hex
                    << std::setw(4)
                    << std::setfill('0')
                    << codePoint;

                result += unicode.str();
            }
            else if (codePoint <= 0x10FFFF)
            {
                /*
                 * Преобразование Unicode code point
                 * в UTF-16 surrogate pair.
                 */

                const std::uint32_t value =
                    codePoint - 0x10000;

                const std::uint32_t high =
                    0xD800 + (value >> 10);

                const std::uint32_t low =
                    0xDC00 + (value & 0x3FF);

                std::ostringstream unicode;

                unicode
                    << "\\U+"
                    << std::uppercase
                    << std::hex
                    << std::setw(4)
                    << std::setfill('0')
                    << high
                    << "\\U+"
                    << std::setw(4)
                    << std::setfill('0')
                    << low;

                result += unicode.str();
            }
            else
            {
                result += '?';
            }
        }

        return result;
    }
}


void DxfDocument::addLayer(
    const DxfLayer& layer)
{
    for (const DxfLayer& existingLayer : layers)
    {
        if (existingLayer.getName() == layer.getName())
        {
            return;
        }
    }

    layers.push_back(layer);
}


void DxfDocument::addCircle(
    const std::string& layerName,
    double x,
    double y,
    double radius)
{
    std::ostringstream entity;

    // Координаты DXF нельзя ограничивать стандартными 6 значащими цифрами
    // ostream: для СК-1963 это давало потерю сантиметров и миллиметров.
    entity
        << "0\n"
        << "CIRCLE\n"
        << "100\n"
        << "AcDbEntity\n"
        << "8\n"
        << layerName << "\n"
        << "100\n"
        << "AcDbCircle\n"
        << "10\n"
        << formatDxfNumber(x) << "\n"
        << "20\n"
        << formatDxfNumber(y) << "\n"
        << "30\n"
        << "0.0\n"
        << "40\n"
        << formatDxfNumber(radius) << "\n";

    entities.push_back(
        {entity.str()});
}


void DxfDocument::addText(
    const std::string& layerName,
    double x,
    double y,
    double height,
    const std::string& text)
{
    /*
     * Преобразуем UTF-8 строку в Unicode escape,
     * понятный AutoCAD.
     */
    const std::string dxfText =
        encodeUtf8ToDxfUnicode(text);

    std::ostringstream entity;

    // Точные координаты и положения подписей сохраняем без округления.
    entity
        << "0\n"
        << "TEXT\n"
        << "100\n"
        << "AcDbEntity\n"
        << "8\n"
        << layerName << "\n"
        << "100\n"
        << "AcDbText\n"
        << "10\n"
        << formatDxfNumber(x) << "\n"
        << "20\n"
        << formatDxfNumber(y) << "\n"
        << "30\n"
        << "0.0\n"
        << "40\n"
        << formatDxfNumber(height) << "\n"
        << "7\n"
        << "STANDARD\n"
        << "1\n"
        << dxfText << "\n";

    entities.push_back(
        {entity.str()});
}


void DxfDocument::addLine(
    const std::string& layerName,
    double x1,
    double y1,
    double x2,
    double y2)
{
    std::ostringstream entity;

    // Сохраняем исходную точность координат линий.
    entity
        << "0\n"
        << "LINE\n"
        << "100\n"
        << "AcDbEntity\n"
        << "8\n"
        << layerName << "\n"
        << "100\n"
        << "AcDbLine\n"
        << "10\n"
        << formatDxfNumber(x1) << "\n"
        << "20\n"
        << formatDxfNumber(y1) << "\n"
        << "30\n"
        << "0.0\n"
        << "11\n"
        << formatDxfNumber(x2) << "\n"
        << "21\n"
        << formatDxfNumber(y2) << "\n"
        << "31\n"
        << "0.0\n";

    entities.push_back(
        {entity.str()});
}


void DxfDocument::addPolyline(
    const std::string& layerName,
    const std::vector<std::pair<double, double>>& points,
    bool closed,
    const std::string& lineType)
{
    if (points.size() < 2)
        return;

    std::ostringstream entity;

    entity
        << "0\n"
        << "LWPOLYLINE\n"
        << "100\n"
        << "AcDbEntity\n"
        << "8\n"
        << layerName << "\n"
        << "6\n"
        << (lineType.empty() ? "BYLAYER" : lineType) << "\n"
        << "100\n"
        << "AcDbPolyline\n"
        << "90\n"
        << points.size() << "\n"
        << "70\n"
        << (closed ? 1 : 0) << "\n";

    for (const auto& [x, y] : points)
    {
        entity
            << "10\n"
            << formatDxfNumber(x)
            << "\n"
            << "20\n"
            << formatDxfNumber(y)
            << "\n";
    }

    entities.push_back({entity.str()});
}


void DxfDocument::save(
    const std::string& filePath) const
{
    std::ofstream file(
        filePath,
        std::ios::out);

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Не удалось создать DXF-файл: " +
            filePath);
    }

    writeHeader(file);

    writeLayers(file);

    writeEntities(file);

    file
        << "0\n"
        << "ENDSEC\n"
        << "0\n"
        << "EOF\n";

    file.close();
}


void DxfDocument::writeHeader(
    std::ofstream& file) const
{
    file
        << "0\n"
        << "SECTION\n"
        << "2\n"
        << "HEADER\n"
        << "0\n"
        << "ENDSEC\n";
}


void DxfDocument::writeLayers(
    std::ofstream& file) const
{
    file
        << "0\n"
        << "SECTION\n"
        << "2\n"
        << "TABLES\n";

    /*
     * ------------------------------------------------------------
     * Типы линий
     * ------------------------------------------------------------
     *
     * DASHED       — штриховая:       — — —
     * DASHDOT      — штрихпунктирная: — · — · —
     * DASHDOTDOT   — штрихпунктирная: — · · — · · —
     *
     * Для C1 используется DASHDOT, для C2 — DASHDOTDOT.
     */
    file
        << "0\n"
        << "TABLE\n"
        << "2\n"
        << "LTYPE\n"
        << "70\n"
        << "4\n";

    // CONTINUOUS
    file
        << "0\n"
        << "LTYPE\n"
        << "2\n"
        << "CONTINUOUS\n"
        << "70\n"
        << "0\n"
        << "3\n"
        << "Solid line\n"
        << "72\n"
        << "65\n"
        << "73\n"
        << "0\n"
        << "40\n"
        << "0.0\n";

    // DASHED
    file
        << "0\n"
        << "LTYPE\n"
        << "2\n"
        << "DASHED\n"
        << "70\n"
        << "0\n"
        << "3\n"
        << "Dashed line\n"
        << "72\n"
        << "65\n"
        << "73\n"
        << "2\n"
        << "40\n"
        << "6.0\n"
        << "49\n"
        << "3.0\n"
        << "49\n"
        << "-3.0\n";

    // DASHDOT — один штрих и одна точка.
    file
        << "0\n"
        << "LTYPE\n"
        << "2\n"
        << "DASHDOT\n"
        << "70\n"
        << "0\n"
        << "3\n"
        << "Dash dot line\n"
        << "72\n"
        << "65\n"
        << "73\n"
        << "4\n"
        << "40\n"
        << "12.0\n"
        << "49\n"
        << "6.0\n"
        << "49\n"
        << "-3.0\n"
        << "49\n"
        << "1.0\n"
        << "49\n"
        << "-3.0\n";

    // DASHDOTDOT — один штрих и две точки.
    file
        << "0\n"
        << "LTYPE\n"
        << "2\n"
        << "DASHDOTDOT\n"
        << "70\n"
        << "0\n"
        << "3\n"
        << "Dash dot dot line\n"
        << "72\n"
        << "65\n"
        << "73\n"
        << "6\n"
        << "40\n"
        << "16.0\n"
        << "49\n"
        << "6.0\n"
        << "49\n"
        << "-3.0\n"
        << "49\n"
        << "1.0\n"
        << "49\n"
        << "-3.0\n"
        << "49\n"
        << "1.0\n"
        << "49\n"
        << "-3.0\n"
        << "0\n"
        << "ENDTAB\n";

    file
        << "0\n"
        << "TABLE\n"
        << "2\n"
        << "STYLE\n"
        << "70\n"
        << "1\n"
        << "0\n"
        << "STYLE\n"
        << "2\n"
        << "STANDARD\n"
        << "70\n"
        << "0\n"
        << "40\n"
        << "0.0\n"
        << "41\n"
        << "1.0\n"
        << "50\n"
        << "0.0\n"
        << "71\n"
        << "0\n"
        << "42\n"
        << "2.5\n"
        << "3\n"
        << "\n"
        << "4\n"
        << "\n"
        << "0\n"
        << "ENDTAB\n";

    file
        << "0\n"
        << "TABLE\n"
        << "2\n"
        << "LAYER\n"
        << "70\n"
        << layers.size()
        << "\n";

    for (const DxfLayer& layer : layers)
    {
        file
            << "0\n"
            << "LAYER\n"
            << "2\n"
            << layer.getName()
            << "\n"
            << "70\n"
            << "0\n"
            << "62\n"
            << layer.getColor()
            << "\n"
            << "6\n"
            << layer.getLineType()
            << "\n";
    }

    file
        << "0\n"
        << "ENDTAB\n"
        << "0\n"
        << "ENDSEC\n";
}


void DxfDocument::writeEntities(
    std::ofstream& file) const
{
    file
        << "0\n"
        << "SECTION\n"
        << "2\n"
        << "ENTITIES\n";

    for (const Entity& entity : entities)
    {
        file << entity.data;
    }
}