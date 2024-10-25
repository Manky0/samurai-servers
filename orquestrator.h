#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void saveToCsv(json data);

void saveToJpeg(json data); 