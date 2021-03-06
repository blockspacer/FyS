// MIT License
//
// Copyright (c) 2019 Quentin Balland
// Repository : https://github.com/FreeYourSoul/FyS
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//         of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
//         to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//         copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
//         copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//         AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#ifndef FYS_SERVICE_CMLSQL_HH
#define FYS_SERVICE_CMLSQL_HH

#include <spdlog/spdlog.h>
#include <Cml.hh>

namespace fys::cache {
    class CmlCopy final : public Cml {
    public:
        ~CmlCopy() override {}
        CmlCopy(const std::string &pathLocalStorage,
                const std::string &pathCopy) : Cml(pathLocalStorage), _copyPathStorage(pathCopy) {
            if (!std::filesystem::exists(_copyPathStorage))
                SPDLOG_ERROR("Path copy does not exist {}", pathCopy);
        }

    protected:
        void createFileInLocalStorage(const CmlKey &cmlKey) override;

    private:
        std::filesystem::path _copyPathStorage;

    };
}


#endif //FYS_SERVICE_CMLSQL_HH
