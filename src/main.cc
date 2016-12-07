#include <cstdlib>
#include <string>
#include <unistd.h>
#include "util/trim.h"
#include "util/dir.h"
#include "counter.h"
#include "reducer.h"
#include "svm_data_formatter.h"
#include "file_format.h"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "ja_JP.UTF-8");

    if (argc <= 5) {
        std::cout
            << "Usage: "
            << argv[0]
            << " adjective antonym concepts(e.g., A,B,C)"
            << " output_dir pattern_files_dir"
            << " < sentences_processed_with_jdepp" << std::endl;
        return 1;
    }

    // read a query (adjective, antonym and concepts)
    std::string adjective = argv[1];    // read adjective
    std::string antonym = argv[2];    // read antonym

    std::vector<std::string> concepts;
    order_concepts::util::splitStringUsing(argv[3], ",", &concepts); // read concepts

    std::string output_dir = argv[4];

    // pattern file path for counter
    const std::string kPatternFilesPath = argv[5]; // "../dataset/ja/count_patterns/ipa";

    // temporary output folder
    order_concepts::util::mkdir(output_dir);

    // count result file
    const std::string kCountedResultFilePath = output_dir + "/counted";

    // counting hints
    order_concepts::Counter counter(concepts, adjective, antonym);
    counter.readPatternFilesInPath(kPatternFilesPath); // pattern files
    order_concepts::Parser parser(order_concepts::Morph::MORPH_TYPE::IPADIC,
                                    "dummy", order_concepts::FileFormat::STDIN);
    counter.count(parser); // count
    counter.save(kCountedResultFilePath);  // save to a file
    std::fflush(stdout);

    // create the folder name for formatted result
    const std::string kFormattedResultFilesPath = output_dir + "/format";

    // formatting reduced hints for ranking-svm and output them as files
    order_concepts::SvmDataFormatter formatter(kCountedResultFilePath);
    formatter.format(kFormattedResultFilesPath);

    return 0;
}
