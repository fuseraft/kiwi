# `faker`

The `faker` package generates realistic fake data for testing, development, and demos. Data is sourced from the [Faker Ruby gem](https://github.com/faker-ruby/faker) locale files.

---

## Quick Start

```kiwi
println faker::name()         # "Delbert Feeney"
println faker::email()        # "chet_nikolaus39@veum.net"
println faker::address()      # "5953 Bartoletti Camp, North Desmond, PA 08818"
println faker::company()      # "Persistent Krajcik Partners"
println faker::sentence()     # "Alias consequatur aut perferendis sit voluptatem."
```

---

## Package Functions

### Name

---

#### `first_name()`

Returns a random first name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A first name. |

---

#### `last_name()`

Returns a random last name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A last name. |

---

#### `name()`

Returns a random full name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A full name (`"First Last"`). |

---

#### `name_with_middle()`

Returns a random full name with a middle name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A full name with middle name (`"First Middle Last"`). |

---

#### `prefix()`

Returns a random name prefix.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A prefix such as `"Mr."`, `"Dr."`, `"Prof."`, etc. |

---

#### `suffix()`

Returns a random name suffix.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A suffix such as `"Jr."`, `"PhD"`, `"Esq."`, etc. |

---

#### `full_name()`

Returns a random full name with prefix.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A prefixed full name (`"Dr. First Last"`). |

---

#### `gender()`

Returns a random gender label.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | One of `"Male"`, `"Female"`, `"Non-binary"`, `"Prefer not to say"`. |

**Example**

```kiwi
println faker::first_name()      # "Delbert"
println faker::last_name()       # "Feeney"
println faker::name()            # "Delbert Feeney"
println faker::name_with_middle() # "Delbert Chet Feeney"
println faker::prefix()          # "Dr."
println faker::suffix()          # "PhD"
println faker::full_name()       # "Dr. Delbert Feeney"
println faker::gender()          # "Male"
```

---

### Internet

---

#### `domain()`

Returns a random domain name built from a last name and TLD.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A domain name such as `"larkin.io"`. |

---

#### `email()`

Returns a random email address.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | An email address such as `"chet_nikolaus39@veum.net"`. |

---

#### `username()`

Returns a random username.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A username such as `"emily_torres234"`. |

---

#### `password(length = 16)`

Returns a random password string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `length` | The password length (default `16`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A random password. |

---

#### `url()`

Returns a random URL.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A URL such as `"https://www.larkin.io"`. |

---

#### `ip()`

Returns a random IPv4 address.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | An IPv4 address such as `"192.168.1.42"`. |

---

#### `ipv6()`

Returns a random IPv6 address.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | An IPv6 address such as `"2001:0db8:85a3:0000:0000:8a2e:0370:7334"`. |

---

#### `mac_address()`

Returns a random MAC address.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A MAC address such as `"a1:b2:c3:d4:e5:f6"`. |

---

#### `slug()`

Returns a random URL slug built from three lorem words.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A URL slug such as `"alias-consequatur-aut"`. |

---

#### `http_method()`

Returns a random HTTP method.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | One of `"GET"`, `"POST"`, `"PUT"`, `"PATCH"`, `"DELETE"`, `"HEAD"`, `"OPTIONS"`. |

---

#### `http_status()`

Returns a random HTTP status code.

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | A status code such as `200`, `404`, or `500`. |

---

#### `mime_type()`

Returns a random MIME type.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A MIME type such as `"application/json"`. |

---

#### `user_agent()`

Returns a random browser user agent string.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A user agent string. |

**Example**

```kiwi
println faker::email()       # "chet_nikolaus39@veum.net"
println faker::username()    # "chet_nikolaus234"
println faker::password()    # "xK3$mNpQ8!vLrZwA"
println faker::password(8)   # "xK3$mNpQ"
println faker::domain()      # "larkin.io"
println faker::url()         # "https://www.larkin.io"
println faker::ip()          # "192.168.1.42"
println faker::ipv6()        # "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
println faker::mac_address() # "a1:b2:c3:d4:e5:f6"
println faker::slug()        # "alias-consequatur-aut"
println faker::http_method() # "GET"
println faker::http_status() # 200
println faker::mime_type()   # "application/json"
```

---

### Phone

---

#### `phone()`

Returns a random US phone number in `(NXX) NXX-XXXX` format.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A formatted phone number such as `"(415) 555-0123"`. |

---

#### `phone_e164()`

Returns a random US phone number in E.164 format.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A phone number such as `"+14155550123"`. |

**Example**

```kiwi
println faker::phone()       # "(914) 201-4866"
println faker::phone_e164()  # "+19142014866"
```

---

### Address

---

#### `street_number()`

Returns a random street number.

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | A street number between 1 and 9999. |

---

#### `street_name()`

Returns a random street name built from a last name and a USPS street suffix.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A street name such as `"Larkin Avenue"`. |

---

#### `street_address()`

Returns a random street address.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A street address such as `"817 Larkin Avenue"`. |

---

#### `secondary_address()`

Returns a random secondary address (apartment, suite, etc.).

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A secondary address such as `"Apt 42"`. |

---

#### `city()`

Returns a random city name, generated by combining city prefixes, first/last names, and city suffixes (matching the Ruby Faker pattern).

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A city name such as `"North Kimberlyville"` or `"Feeneyport"`. |

---

#### `state()`

Returns a random US state name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A state name such as `"Colorado"`. |

---

#### `state_abbr()`

Returns a random US state abbreviation.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A state abbreviation such as `"CO"`. |

---

#### `zip()`

Returns a random 5-digit US ZIP code.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A ZIP code such as `"80201"`. |

---

#### `country()`

Returns a random country name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A country name such as `"Germany"`. |

---

#### `address()`

Returns a random US-style address with street, city, state, and ZIP.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | An address such as `"817 Larkin Avenue, North Kimberlyville, CO 80201"`. |

---

#### `full_address()`

Returns a random full address including country.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A full address such as `"817 Larkin Avenue, North Kimberlyville, CO 80201, Germany"`. |

---

#### `latitude()`

Returns a random latitude value.

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | A latitude between `-90.0` and `90.0`. |

---

#### `longitude()`

Returns a random longitude value.

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | A longitude between `-180.0` and `180.0`. |

**Example**

```kiwi
println faker::street_address()   # "817 Larkin Avenue"
println faker::secondary_address() # "Apt 42"
println faker::city()             # "North Kimberlyville"
println faker::state()            # "Colorado"
println faker::state_abbr()       # "CO"
println faker::zip()              # "80201"
println faker::country()          # "Germany"
println faker::address()          # "817 Larkin Avenue, North Kimberlyville, CO 80201"
println faker::full_address()     # "817 Larkin Avenue, North Kimberlyville, CO 80201, Germany"
println faker::latitude()         # 39.74
println faker::longitude()        # -104.98
```

---

### Company

---

#### `company()`

Returns a random company name built from a buzzword adjective, a last name, and a suffix.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A company name such as `"Adaptive Larkin Inc"`. |

---

#### `company_suffix()`

Returns a random company name suffix.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A suffix such as `"Inc"`, `"LLC"`, `"Corp"`, etc. |

---

#### `department()`

Returns a random department name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A department such as `"Engineering"` or `"Human Resources"`. |

---

#### `job()`

Returns a random job title composed of a seniority level and a position.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A job title such as `"Senior Engineer"` or `"Principal Analyst"`. |

---

#### `profession()`

Returns a random profession.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A profession such as `"architect"` or `"pharmacist"`. |

**Example**

```kiwi
println faker::company()        # "Adaptive Larkin Inc"
println faker::company_suffix() # "LLC"
println faker::department()     # "Engineering"
println faker::job()            # "Senior Engineer"
println faker::profession()     # "architect"
```

---

### Lorem

---

#### `word()`

Returns a random lorem ipsum word.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A word such as `"consequatur"`. |

---

#### `words(n = 3)`

Returns a list of random lorem ipsum words.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `n` | The number of words (default `3`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of word strings. |

---

#### `sentence(word_count = 7)`

Returns a random lorem ipsum sentence.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `word_count` | The number of words (default `7`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A capitalized sentence ending with `"."`. |

---

#### `sentences(count = 3)`

Returns a list of random lorem ipsum sentences.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `count` | The number of sentences (default `3`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of sentence strings. |

---

#### `paragraph(sentence_count = 4)`

Returns a random lorem ipsum paragraph.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `sentence_count` | The number of sentences (default `4`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A paragraph of lorem ipsum text. |

---

#### `paragraphs(count = 3)`

Returns a list of random lorem ipsum paragraphs.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `count` | The number of paragraphs (default `3`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of paragraph strings. |

**Example**

```kiwi
println faker::word()          # "consequatur"
println faker::words(5)        # ["alias", "aut", "dolorem", "sit", "velit"]
println faker::sentence()      # "Alias consequatur aut perferendis sit voluptatem accusantium."
println faker::sentence(4)     # "Alias consequatur aut perferendis."
println faker::paragraph()     # "Alias consequatur aut... Doloremque aperiam eaque..."
println faker::paragraph(2)    # Two-sentence paragraph.
```

---

### Number

---

#### `digit()`

Returns a random single digit.

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | An integer from `0` to `9`. |

---

#### `number(digits = 10)`

Returns a random integer with a given number of digits.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `digits` | The number of digits (default `10`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | A random integer. |

---

#### `between(min, max)`

Returns a random integer between `min` and `max` (inclusive).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `min` | The lower bound. |
| `integer` | `max` | The upper bound. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | A random integer in `[min, max]`. |

---

#### `float_between(min = 0.0, max = 1.0)`

Returns a random float between `min` and `max`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `min` | The lower bound (default `0.0`). |
| `float` | `max` | The upper bound (default `1.0`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | A random float in `[min, max]`. |

---

#### `positive(max = 1000)`

Returns a random positive integer.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `max` | The upper bound (default `1000`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | A positive integer in `[1, max]`. |

---

#### `negative(min = -1000)`

Returns a random negative integer.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `min` | The lower bound (default `-1000`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | A negative integer in `[min, -1]`. |

---

#### `percentage()`

Returns a random percentage value.

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | An integer from `0` to `100`. |

---

#### `rand_bool()`

Returns a random boolean value.

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` or `false`. |

**Example**

```kiwi
println faker::digit()            # 7
println faker::number()           # 4823901654
println faker::number(4)          # 3821
println faker::between(1, 100)    # 42
println faker::float_between()    # 0.7312...
println faker::float_between(1.0, 9.99) # 5.32
println faker::positive()         # 662
println faker::positive(50)       # 37
println faker::negative()         # -340
println faker::negative(-10)      # -7
println faker::percentage()       # 83
println faker::rand_bool()        # true
```

---

### Color

---

#### `color()`

Returns a random color name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A color name such as `"crimson"` or `"cerulean"`. |

---

#### `hex_color()`

Returns a random hex color code.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A hex color such as `"#a1b2c3"`. |

---

#### `rgb_color()`

Returns a random RGB color string.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | An RGB color such as `"rgb(100, 200, 50)"`. |

**Example**

```kiwi
println faker::color()      # "cerulean"
println faker::hex_color()  # "#5a8f14"
println faker::rgb_color()  # "rgb(197, 74, 56)"
```

---

### File

---

#### `file_extension()`

Returns a random file extension.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A file extension such as `"pdf"` or `"kiwi"`. |

---

#### `filename()`

Returns a random filename composed of a lorem word, 4 random digits, and an extension.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A filename such as `"alias_4821.pdf"`. |

**Example**

```kiwi
println faker::file_extension() # "pdf"
println faker::filename()       # "alias_4821.pdf"
```

---

### Finance

---

#### `currency()`

Returns a random currency code.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A currency code such as `"USD"`. |

---

#### `currency_symbol()`

Returns a random currency symbol.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A currency symbol such as `"$"`. |

---

#### `price(min = 1.0, max = 999.99)`

Returns a random price rounded to 2 decimal places.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `min` | The minimum price (default `1.0`). |
| `float` | `max` | The maximum price (default `999.99`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | A price such as `42.95`. |

---

#### `credit_card()`

Returns a random 16-digit credit card number formatted in groups of 4.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A card number such as `"4532 1234 5678 9012"`. |

**Example**

```kiwi
println faker::currency()        # "USD"
println faker::currency_symbol() # "$"
println faker::price()           # 707.44
println faker::price(5.0, 50.0)  # 23.17
println faker::credit_card()     # "4267 4412 9674 8241"
```

---

### Miscellaneous

---

#### `uuid()`

Returns a random UUID (version 4).

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A UUID such as `"0f07115e-7633-48c2-8a0c-a24a8e816df2"`. |

---

#### `language()`

Returns a random programming language name.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A language name such as `"Rust"` or `"Kiwi"`. |

---

#### `hashtag()`

Returns a random lorem ipsum hashtag.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A hashtag such as `"#alias"`. |

---

#### `person()`

Generates a hashmap of fake person data.

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap with `name`, `email`, `phone`, `address`, `company`, `job`, and `profession` keys. |

---

#### `people(count = 10)`

Generates a list of fake person hashmaps.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `count` | The number of people to generate (default `10`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of person hashmaps. |

**Example**

```kiwi
println faker::uuid()     # "0f07115e-7633-48c2-8a0c-a24a8e816df2"
println faker::language() # "Kiwi"
println faker::hashtag()  # "#alias"

p = faker::person()
println p["name"]    # "Delbert Feeney"
println p["email"]   # "chet_nikolaus39@veum.net"
println p["phone"]   # "(914) 201-4866"
println p["address"] # "5953 Bartoletti Camp, North Desmond, PA 08818"
println p["company"] # "Persistent Krajcik Partners"
println p["job"]     # "Future Specialist"

ppl = faker::people(3)
for person in ppl do
  n = person["name"]
  e = person["email"]
  println "${n} <${e}>"
end
# Antwan Boyle <craig_goldner70@mclaughlin.org>
# Aubrey Nolan <blair.heller11@hartmann.net>
# Cornell Howe <beau_dooley96@reichert.org>
```
