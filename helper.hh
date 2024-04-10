#include <stdexcept>
#include <string>
#include <variant>

int convertToInt(const std::variant<int, double, bool, std::string> &value)
{
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    } else if (std::holds_alternative<double>(value)) {
        // Convert double to int
        return static_cast<int>(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        // Convert string to int if possible
        try {
            return std::stoi(std::get<std::string>(value));
        } catch (const std::invalid_argument &) {
            // Conversion failed, handle error or return default value
            throw std::invalid_argument("Failed to convert string to int");
        } catch (const std::out_of_range &) {
            // Conversion resulted in out of range, handle error or return default value
            throw std::out_of_range("Out of range error occurred during string to int conversion");
        }
    } else {
        // Variant does not contain a valid type for conversion to int
        throw std::invalid_argument("Variant does not contain a valid type for conversion to int");
    }
}
