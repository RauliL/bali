#include <cstdio>
#include <cstdlib>
#include <fstream>

#if defined(_WIN32)
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include <peelo/prompt.hpp>

#include <bali/error.hpp>
#include <bali/eval.hpp>
#include <bali/parser.hpp>

static std::string programfile;
static bool use_mexpression = false;

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
  if (count < 0)
  {
    count = 0;
  }
}

static void
repl(const std::shared_ptr<bali::scope>& scope)
{
  peelo::prompt prompt;
  std::string script;
  int line_counter = 0;
  int parenthesis_counter = 0;

  while (const auto line = prompt.input(
    "bali:" +
    std::to_string(line_counter) +
    ":" +
    std::to_string(parenthesis_counter) +
    "> ")
  )
  {
    script += *line;
    ++line_counter;
    prompt.add_to_history(*line);
    count_open_parenthesis(*line, parenthesis_counter);
    if (parenthesis_counter == 0)
    {
      try
      {
        for (const auto& value : bali::parse(
          script,
          line_counter,
          1,
          use_mexpression
        ))
        {
          std::cout << bali::eval(value, scope) << std::endl;
        }
      }
      catch (bali::error& e)
      {
        std::cout << e << std::endl;
      }
      catch (bali::function_return&)
      {
        std::cout << "Unexpected `return'." << std::endl;
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

  try
  {
    for (const auto& value : bali::parse(source, 1, 1, use_mexpression))
    {
      bali::eval(value, scope);
    }
  }
  catch (bali::error& e)
  {
    std::cerr << e << std::endl;
    std::exit(EXIT_FAILURE);
  }
  catch (bali::function_return&)
  {
    std::cerr << "Unexpected `return'." << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

static void
print_usage(std::ostream& output, const char* executable_name)
{
  output
    << std::endl
    << "Usage: "
    << executable_name
    << " [switches] [programfile]"
    << std::endl
    << "  -m                Use M-expressions."
    << std::endl
    << "  --version         Print the version."
    << std::endl
    << "  --help            Display this message."
    << std::endl
    << std::endl;
}

static void
parse_args(int argc, char** argv)
{
  int offset = 1;

  while (offset < argc)
  {
    auto arg = argv[offset++];

    if (!*arg)
    {
      continue;
    }
    else if (*arg != '-')
    {
      programfile = arg;
      break;
    }
    else if (!arg[1])
    {
      break;
    }
    else if (arg[1] == '-')
    {
      if (!std::strcmp(arg, "--help"))
      {
        print_usage(std::cout, argv[0]);
        std::exit(EXIT_SUCCESS);
      }
      else if (!std::strcmp(arg, "--version"))
      {
        std::cerr << "Bali 1.0" << std::endl;
        std::exit(EXIT_SUCCESS);
      } else {
        std::cerr << "Unrecognized switch: " << arg << std::endl;
        print_usage(std::cerr, argv[0]);
        std::exit(EXIT_FAILURE);
      }
    }
    for (int i = 1; arg[i]; ++i)
    {
      switch (arg[i])
      {
        case 'm':
          use_mexpression = true;
          break;

        case 'h':
          print_usage(std::cout, argv[0]);
          std::exit(EXIT_SUCCESS);
          break;

        default:
          std::cerr << "Unrecognized switch: `" << arg[i] << "'" << std::endl;
          std::exit(EXIT_FAILURE);
          break;
      }
    }
  }

  if (offset < argc)
  {
    std::cerr << "Too many arguments given." << std::endl;
    print_usage(std::cerr, argv[0]);
    std::exit(EXIT_FAILURE);
  }
}

static inline bool
is_interactive_console()
{
#if defined(_WIN32)
  return _isatty(_fileno(stdin));
#else
  return isatty(fileno(stdin));
#endif
}

int
main(int argc, char** argv)
{
  auto scope = std::make_shared<bali::scope>();

  parse_args(argc, argv);

  if (!programfile.empty())
  {
    std::ifstream file(programfile);

    if (!file.good())
    {
      std::cerr
        << argv[0]
        << ": Unable to open file `"
        << programfile
        << "'"
        << std::endl;
      std::exit(EXIT_FAILURE);
    }
    run_file(file, scope);
    file.close();
  }
  else if (is_interactive_console())
  {
    repl(scope);
  } else {
    run_file(std::cin, scope);
  }

  return EXIT_SUCCESS;
}
