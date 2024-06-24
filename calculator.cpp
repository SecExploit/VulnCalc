#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <fcgi_stdio.h>

const char *html_template = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>Calculator</title>
</head>
<body>
    <h1>Simple Calculator</h1>
    <input type="text" id="expression" placeholder="Enter expression">
    <button onclick="calculate()">Calculate</button>
    <p>Result: <span id="result"></span></p>
    <script>
        function calculate() {
            const expression = document.getElementById('expression').value;
            fetch(`/calculator?expr=${encodeURIComponent(expression)}`)
                .then(response => response.json())
                .then(data => {
                    document.getElementById('result').innerText = data.result;
                })
                .catch(error => {
                    document.getElementById('result').innerText = 'Error';
                });
        }
    </script>
</body>
</html>
)HTML";

int main() {
    while (FCGI_Accept() >= 0) {
        std::string request_uri = getenv("REQUEST_URI");

        if (request_uri == "/") {
            std::cout << "Status: 200 OK\r\n"
                      << "Content-Type: text/html\r\n\r\n"
                      << html_template;
        } else if (request_uri.find("/calculator?expr=") != std::string::npos) {
            std::string query_string = getenv("QUERY_STRING");
            std::string expr = query_string.substr(query_string.find("expr=") + 5);
            std::string command = "echo " + expr + " | bc";
            
            FILE *fp = popen(command.c_str(), "r");
            if (fp == NULL) {
                std::cout << "Status: 500 Internal Server Error\r\n"
                          << "Content-Type: text/html\r\n\r\n"
                          << "<html><body><h1>500 Internal Server Error</h1></body></html>";
                continue;
            }

            char buffer[128];
            std::string result = "";
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                result += buffer;
            }
            pclose(fp);

            std::cout << "Status: 200 OK\r\n"
                      << "Content-Type: application/json\r\n\r\n"
                      << "{\"result\": \"" << result << "\"}";
        }
    }
    return 0;
}
