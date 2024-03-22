#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <unistd.h>

#include <peelo/prompt.hpp>

#include <bali/error.hpp>
#include <bali/eval.hpp>
#include <bali/parser.hpp>

static void
count_open_parenthesis(const std::string& input, int& count)
{
  for (const auto& c : input)
  {
    if (c == '(')
    {
      ++count;
    }
    else if (c == ')')
    {
      --count;
    }
  }
}

static void
repl(const std::shared_ptr<bali::scope>& scope)
{
  peelo::prompt prompt;
  std::string script;
  int line_counter = 0;
  int parenthesis_counter = 0;

  while (const auto line = prompt.input("> "))
  {
    script += *line;
    ++line_counter;
    prompt.add_to_history(*line);
    count_open_parenthesis(*line, parenthesis_counter);
    if (parenthesis_counter == 0)
    {
      bali::parser parser(script, line_counter);

      try
      {
        for (const auto& value : parser.parse())
        {
          std::cout << bali::eval(value, scope) << std::endl;
        }
      }
      catch (bali::error& e)
      {
        std::cout << e << std::endl;
      }
      script.clear();
    }
  }
}

static void
run_file(
  std::istream& file,
  const std::shared_ptr<bali::scope>& scope
)
{
  const auto source = std::string(
    std::istreambuf_iterator<char>(file),
    std::istreambuf_iterator<char>()
  );
  bali::parser parser(source);

  try
  {
    for (const auto& value : parser.parse())
    {
      bali::eval(value, scope);
    }
  }
  catch (bali::error& e)
  {
    std::cerr << e << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

int
main(int argc, char** argv)
{
  auto scope = std::make_shared<bali::scope>();

  if (argc > 2)
  {
    std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  else if (argc == 2)
  {
    std::ifstream file(argv[1]);

    if (!file.good())
    {
      std::cerr
        << argv[0]
        << ": Unable to open file `"
        << argv[1]
        << "'"
        << std::endl;
      std::exit(EXIT_FAILURE);
    }
    run_file(file, scope);
    file.close();
  }
  else if (isatty(fileno(stdin)))
  {
    repl(scope);
  } else {
    run_file(std::cin, scope);
  }

  return EXIT_SUCCESS;
}
