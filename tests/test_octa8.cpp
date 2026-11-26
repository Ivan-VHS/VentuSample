#include "octa8.hpp"
#include <cassert>
#include <limits>
#include <memory>
#include <string>

int main() {
    using namespace ventu;
    Octa8Value null_value{};
    assert(octa8_validate(null_value));
    Octa8Value boolean_value{true};
    assert(boolean_value.kind() == Octa8Kind::Boolean && octa8_validate(boolean_value));
    Octa8Value number_value{3.5};
    assert(octa8_validate(number_value));
    Octa8Value invalid_number{std::numeric_limits<double>::infinity()};
    assert(!octa8_validate(invalid_number));

    auto sequence = std::make_shared<Octa8Sequence>();
    sequence->push_back(Octa8Value{std::int64_t{12}});
    sequence->push_back(Octa8Value{std::string("sample")});
    Octa8Value sequence_value{sequence};
    assert(sequence_value.kind() == Octa8Kind::Sequence && octa8_validate(sequence_value));

    auto record = std::make_shared<Octa8Record>();
    (*record)["items"] = sequence_value;
    Octa8Value record_value{record};
    assert(record_value.kind() == Octa8Kind::Record && octa8_validate(record_value));
    assert(octa8_kind_name(Octa8Kind::Record) == "record");
    return 0;
}
