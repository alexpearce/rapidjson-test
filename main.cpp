#include <iostream>
#include <string>

#include "rapidjson/document.h"

std::string exec(const char* cmd)
{
  FILE* pipe = popen(cmd, "r");
  if (!pipe) {
    return "ERROR";
  }
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL) {
      result += buffer;
    }
  }
  pclose(pipe);
  return result;
}

/* Get the run view configuration and parse and display it. */
void config()
{
  const char* cmd = "python ./run_view_config.py config";
  // Get the JSON string and parse it
  std::string json = exec(cmd);
  rapidjson::Document document;
  document.Parse(json.c_str());

  for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
       itr != document.MemberEnd(); ++itr) {
    std::string tabName = itr->name.GetString();
    const rapidjson::Value &tab = itr->value;
    std::cout << tabName << " :: " << tab["title"].GetString();
    if (!tab.HasMember("plots")) {
      std::cout << " :: No plots found" << std::endl << std::endl;
      continue;
    }
    const rapidjson::Value &plots = tab["plots"];
    rapidjson::SizeType numPlots = plots.Size();
    std::cout << " :: " << numPlots << " plots found" << std::endl;
    for (rapidjson::SizeType i = 0; i < plots.Size(); ++i) {
      const rapidjson::Value &plot = plots[i];
      std::string plotName = plot["name"].GetString();
      std::string plotTitle = plot["title"].GetString();
      std::cout << plotTitle << " :: " << plotName;
      bool sensorDependent = false;
      if (plot.HasMember("sensor_dependent") && plot["sensor_dependent"].IsBool()) {
        sensorDependent = plot["sensor_dependent"].GetBool();
        std::cout << " :: Sensor dependent";
      } else {
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

/* Get a plot and collate some information. */
void plot()
{
  const char* cmd = "python ./run_view_config.py plot";
  // Get the JSON string and parse it
  std::string json = exec(cmd);
  rapidjson::Document document;
  document.Parse(json.c_str());

  bool success = false;
  if (document.HasMember("success")) {
    success = document["success"].GetBool();
  }
  if (!success) {
    std::cout << "Data retrieved unsuccessfully" << std::endl;
    return;
  }

  rapidjson::Value &data = document["data"];
  std::string objClass = data["object_class"].GetString();
  std::string objName = data["name"].GetString();
  std::string objTitle = data["title"].GetString();
  std::cout << objClass << " :: " << objName << " :: " << objTitle << std::endl;

  rapidjson::Value &plotData = data["data"];
  rapidjson::Value &vals = plotData["values"];
  rapidjson::Value &errs = plotData["uncertainties"];
  rapidjson::Value &bins = plotData["binning"];
  if (!(vals.Size() == errs.Size() && errs.Size() == bins.Size())) {
    std::cout << "Values/uncertainties/binning of different lengths" << std::endl;
    std::cout << "Val size " << vals.Size() << std::endl;
    std::cout << "Err size " << errs.Size() << std::endl;
    std::cout << "Bin size " << bins.Size() << std::endl;
  }

  double entries = plotData["entries"].GetDouble();
  double mean = plotData["mean"].GetDouble();
  double rms = plotData["rms"].GetDouble();
  double underflow = plotData["underflow"].GetDouble();
  double overflow = plotData["overflow"].GetDouble();

  std::string xAxisTitle = plotData["axis_titles"][0].GetString();
  std::string yAxisTitle = plotData["axis_titles"][1].GetString();
}

int main( int argc, const char* argv[] )
{
  config();
  plot();

  return 0;
}
