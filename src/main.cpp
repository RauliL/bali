#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <unistd.h>

#include <peelo/prompt.hpp>

#include <lisp/error.hpp>
#include <lisp/parser.hpp>

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
repl()
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
      lisp::parser parser(script, line_counter);

      try
      {
        for (const auto& value : parser.parse())
        {
          std::cout << lisp::eval(value) << std::endl;
        }
      }
      catch (lisp::error& e)
      {
        std::cout << e << std::endl;
      }
      script.clear();
    }
  }
}

static void
run_file(std::istream& file)
{
  const auto source = std::string(
    std::istreambuf_iterator<char>(file),
    std::istreambuf_iterator<char>()
  );
  lisp::parser parser(source);

  try
  {
    for (const auto& value : parser.parse())
    {
      lisp::eval(value);
    }
  }
  catch (lisp::error& e)
  {
    std::cerr << e << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

int
main(int argc, char** argv)
{
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
    run_file(file);
    file.close();
  }
  else if (isatty(fileno(stdin)))
  {
    repl();
  } else {
    run_file(std::cin);
  }

  return EXIT_SUCCESS;
}
