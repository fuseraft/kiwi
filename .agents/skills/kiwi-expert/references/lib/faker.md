# faker

> Generate realistic fake data for tests, demos and seed files.

The `faker` package generates random but realistic-looking data: names, emails, addresses, companies, text, numbers and more. Every call returns a new random value.

```kiwi
println faker::name()       # Delbert Feeney
println faker::email()      # chet_nikolaus39@veum.net
println faker::address()    # 5953 Bartoletti Camp, North Desmond, PA 08818
println faker::company()    # Persistent Krajcik Partners
println faker::sentence()   # Alias consequatur aut perferendis sit voluptatem.
```

`person()` returns a whole record, and `people(n)` a list of them, which makes seeding test data quick:

```kiwi
someone = faker::person()
println someone.keys()   # ["name", "email", "phone", "address", "company", "job", "profession"]

for p in faker::people(3) do
  println "${p["name"]} <${p["email"]}>"
end
```

## What it generates

| Category | Functions |
| :-- | :-- |
| Names | `first_name`, `last_name`, `name`, `name_with_middle`, `full_name`, `prefix`, `suffix`, `gender` |
| Internet | `email`, `username`, `password`, `domain`, `url`, `ip`, `ipv6`, `mac_address`, `slug`, `user_agent`, `http_method`, `http_status`, `mime_type` |
| Phone | `phone`, `phone_e164` |
| Address | `street_number`, `street_name`, `street_address`, `secondary_address`, `city`, `state`, `state_abbr`, `zip`, `country`, `address`, `full_address`, `latitude`, `longitude` |
| Company | `company`, `company_suffix`, `department`, `job`, `profession` |
| Text | `word`, `words`, `sentence`, `sentences`, `paragraph`, `paragraphs`, `hashtag` |
| Numbers | `digit`, `number`, `between`, `float_between`, `positive`, `negative`, `percentage`, `rand_bool` |
| Color | `color`, `hex_color`, `rgb_color` |
| Files | `filename`, `file_extension` |
| Finance | `currency`, `currency_symbol`, `price`, `credit_card` |
| Other | `uuid`, `language`, `person`, `people` |

Word lists come from the locale files of the [Faker Ruby gem](https://github.com/faker-ruby/faker).

## Functions

### `faker::first_name`

```kiwi
faker::first_name()
```

Returns a random first name.

**Returns** `string` — A first name.

### `faker::last_name`

```kiwi
faker::last_name()
```

Returns a random last name.

**Returns** `string` — A last name.

### `faker::name`

```kiwi
faker::name()
```

Returns a random full name.

**Returns** `string` — A full name (`"First Last"`).

### `faker::name_with_middle`

```kiwi
faker::name_with_middle()
```

Returns a random full name with a middle name.

**Returns** `string` — A full name with middle name (`"First Middle Last"`).

### `faker::prefix`

```kiwi
faker::prefix()
```

Returns a random name prefix.

**Returns** `string` — A prefix such as `"Mr."`, `"Dr."`, etc.

### `faker::suffix`

```kiwi
faker::suffix()
```

Returns a random name suffix.

**Returns** `string` — A suffix such as `"Jr."`, `"PhD"`, etc.

### `faker::full_name`

```kiwi
faker::full_name()
```

Returns a random full name with prefix.

**Returns** `string` — A prefixed full name (`"Dr. First Last"`).

### `faker::gender`

```kiwi
faker::gender()
```

Returns a random gender label.

**Returns** `string` — One of `"Male"`, `"Female"`, `"Non-binary"`, `"Prefer not to say"`.

### `faker::domain`

```kiwi
faker::domain()
```

Returns a random domain name.

**Returns** `string` — A domain name such as `"larkin.io"`.

### `faker::email`

```kiwi
faker::email()
```

Returns a random email address.

**Returns** `string` — An email address such as `"emily.torres42@larkin.io"`.

### `faker::username`

```kiwi
faker::username()
```

Returns a random username.

**Returns** `string` — A username such as `"emily_torres234"`.

### `faker::password`

```kiwi
faker::password(length = 16)
```

Returns a random password.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `length` | `integer` | `16` | The password length (default `16`). |

**Returns** `string` — A random password string.

### `faker::url`

```kiwi
faker::url()
```

Returns a random URL.

**Returns** `string` — A URL such as `"https://www.larkin.io"`.

### `faker::ip`

```kiwi
faker::ip()
```

Returns a random IPv4 address.

**Returns** `string` — An IPv4 address such as `"192.168.1.42"`.

### `faker::ipv6`

```kiwi
faker::ipv6()
```

Returns a random IPv6 address.

**Returns** `string` — An IPv6 address such as `"2001:0db8:85a3:0000:0000:8a2e:0370:7334"`.

### `faker::mac_address`

```kiwi
faker::mac_address()
```

Returns a random MAC address.

**Returns** `string` — A MAC address such as `"a1:b2:c3:d4:e5:f6"`.

### `faker::slug`

```kiwi
faker::slug()
```

Returns a random URL slug.

**Returns** `string` — A URL slug such as `"lorem-ipsum-dolor"`.

### `faker::http_method`

```kiwi
faker::http_method()
```

Returns a random HTTP method.

**Returns** `string` — An HTTP method such as `"GET"` or `"POST"`.

### `faker::http_status`

```kiwi
faker::http_status()
```

Returns a random HTTP status code.

**Returns** `integer` — An HTTP status code such as `200` or `404`.

### `faker::mime_type`

```kiwi
faker::mime_type()
```

Returns a random MIME type.

**Returns** `string` — A MIME type such as `"application/json"`.

### `faker::user_agent`

```kiwi
faker::user_agent()
```

Returns a random user agent string.

**Returns** `string` — A browser user agent string.

### `faker::phone`

```kiwi
faker::phone()
```

Returns a random US phone number in `(NXX) NXX-XXXX` format.

**Returns** `string` — A formatted phone number such as `"(415) 555-0123"`.

### `faker::phone_e164`

```kiwi
faker::phone_e164()
```

Returns a random US phone number in E.164 format.

**Returns** `string` — A phone number such as `"+14155550123"`.

### `faker::street_number`

```kiwi
faker::street_number()
```

Returns a random street number.

**Returns** `integer` — A street number between 1 and 9999.

### `faker::street_name`

```kiwi
faker::street_name()
```

Returns a random street name.

**Returns** `string` — A street name such as `"Larkin Avenue"`.

### `faker::street_address`

```kiwi
faker::street_address()
```

Returns a random street address.

**Returns** `string` — A street address such as `"817 Larkin Avenue"`.

### `faker::secondary_address`

```kiwi
faker::secondary_address()
```

Returns a random secondary address (apartment, suite, etc.).

**Returns** `string` — A secondary address such as `"Apt 42"`.

### `faker::city`

```kiwi
faker::city()
```

Returns a random city name.

**Returns** `string` — A city name such as `"North Kimberlyville"`.

### `faker::state`

```kiwi
faker::state()
```

Returns a random US state name.

**Returns** `string` — A state name such as `"Colorado"`.

### `faker::state_abbr`

```kiwi
faker::state_abbr()
```

Returns a random US state abbreviation.

**Returns** `string` — A state abbreviation such as `"CO"`.

### `faker::zip`

```kiwi
faker::zip()
```

Returns a random US ZIP code.

**Returns** `string` — A 5-digit ZIP code such as `"80201"`.

### `faker::country`

```kiwi
faker::country()
```

Returns a random country name.

**Returns** `string` — A country name such as `"Germany"`.

### `faker::address`

```kiwi
faker::address()
```

Returns a random US street address with city, state, and ZIP.

**Returns** `string` — An address such as `"817 Larkin Avenue, North Kimberlyville, CO 80201"`.

### `faker::full_address`

```kiwi
faker::full_address()
```

Returns a random full address including country.

**Returns** `string` — A full address including country.

### `faker::latitude`

```kiwi
faker::latitude()
```

Returns a random latitude value.

**Returns** `float` — A latitude between -90.0 and 90.0.

### `faker::longitude`

```kiwi
faker::longitude()
```

Returns a random longitude value.

**Returns** `float` — A longitude between -180.0 and 180.0.

### `faker::company`

```kiwi
faker::company()
```

Returns a random company name.

**Returns** `string` — A company name such as `"Adaptive Lueilwitz Inc"`.

### `faker::company_suffix`

```kiwi
faker::company_suffix()
```

Returns a random company name suffix.

**Returns** `string` — A suffix such as `"Inc"` or `"LLC"`.

### `faker::department`

```kiwi
faker::department()
```

Returns a random department name.

**Returns** `string` — A department such as `"Engineering"`.

### `faker::job`

```kiwi
faker::job()
```

Returns a random job title.

**Returns** `string` — A job title such as `"Senior Engineer"`.

### `faker::profession`

```kiwi
faker::profession()
```

Returns a random profession.

**Returns** `string` — A profession such as `"architect"`.

### `faker::word`

```kiwi
faker::word()
```

Returns a random lorem ipsum word.

**Returns** `string` — A word such as `"consequatur"`.

### `faker::words`

```kiwi
faker::words(n = 3)
```

Returns a list of random lorem ipsum words.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` | `3` | The number of words (default `3`). |

**Returns** `list` — A list of word strings.

### `faker::sentence`

```kiwi
faker::sentence(word_count = 7)
```

Returns a random lorem ipsum sentence.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `word_count` | `integer` | `7` | The number of words (default `7`). |

**Returns** `string` — A sentence ending with `"."`.

### `faker::sentences`

```kiwi
faker::sentences(count = 3)
```

Returns a list of random lorem ipsum sentences.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `count` | `integer` | `3` | The number of sentences (default `3`). |

**Returns** `list` — A list of sentence strings.

### `faker::paragraph`

```kiwi
faker::paragraph(sentence_count = 4)
```

Returns a random lorem ipsum paragraph.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sentence_count` | `integer` | `4` | The number of sentences (default `4`). |

**Returns** `string` — A paragraph of lorem ipsum text.

### `faker::paragraphs`

```kiwi
faker::paragraphs(count = 3)
```

Returns a list of random lorem ipsum paragraphs.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `count` | `integer` | `3` | The number of paragraphs (default `3`). |

**Returns** `list` — A list of paragraph strings.

### `faker::digit`

```kiwi
faker::digit()
```

Returns a random single digit.

**Returns** `integer` — An integer from 0 to 9.

### `faker::number`

```kiwi
faker::number(digits = 10)
```

Returns a random integer with a given number of digits.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `digits` | `integer` | `10` | The number of digits (default `10`). |

**Returns** `integer` — A random integer.

### `faker::between`

```kiwi
faker::between(min, max)
```

Returns a random integer between min and max (inclusive).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `min` | `integer` |  | The lower bound. |
| `max` | `integer` |  | The upper bound. |

**Returns** `integer` — A random integer in `[min, max]`.

### `faker::float_between`

```kiwi
faker::float_between(min = 0.0, max = 1.0)
```

Returns a random float between min and max.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `min` | `float` | `0.0` | The lower bound (default `0.0`). |
| `max` | `float` | `1.0` | The upper bound (default `1.0`). |

**Returns** `float` — A random float in `[min, max]`.

### `faker::positive`

```kiwi
faker::positive(max = 1000)
```

Returns a random positive integer.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `max` | `integer` | `1000` | The upper bound (default `1000`). |

**Returns** `integer` — A positive integer in `[1, max]`.

### `faker::negative`

```kiwi
faker::negative(min = -1000)
```

Returns a random negative integer.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `min` | `integer` | `-1000` | The lower bound (default `-1000`). |

**Returns** `integer` — A negative integer in `[min, -1]`.

### `faker::percentage`

```kiwi
faker::percentage()
```

Returns a random percentage value.

**Returns** `integer` — An integer from 0 to 100.

### `faker::rand_bool`

```kiwi
faker::rand_bool()
```

Returns a random boolean value.

**Returns** `boolean` — `true` or `false`.

### `faker::color`

```kiwi
faker::color()
```

Returns a random color name.

**Returns** `string` — A color name such as `"crimson"`.

### `faker::hex_color`

```kiwi
faker::hex_color()
```

Returns a random hex color code.

**Returns** `string` — A hex color such as `"#a1b2c3"`.

### `faker::rgb_color`

```kiwi
faker::rgb_color()
```

Returns a random RGB color string.

**Returns** `string` — An RGB color such as `"rgb(100, 200, 50)"`.

### `faker::file_extension`

```kiwi
faker::file_extension()
```

Returns a random file extension.

**Returns** `string` — A file extension such as `"pdf"`.

### `faker::filename`

```kiwi
faker::filename()
```

Returns a random filename.

**Returns** `string` — A filename such as `"alias_4821.pdf"`.

### `faker::currency`

```kiwi
faker::currency()
```

Returns a random currency code.

**Returns** `string` — A currency code such as `"USD"`.

### `faker::currency_symbol`

```kiwi
faker::currency_symbol()
```

Returns a random currency symbol.

**Returns** `string` — A currency symbol such as `"$"`.

### `faker::price`

```kiwi
faker::price(min = 1.0, max = 999.99)
```

Returns a random price value rounded to 2 decimal places.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `min` | `float` | `1.0` | The minimum price (default `1.0`). |
| `max` | `float` | `999.99` | The maximum price (default `999.99`). |

**Returns** `float` — A price such as `42.95`.

### `faker::credit_card`

```kiwi
faker::credit_card()
```

Returns a random credit card number (16 digits, formatted in groups of 4).

**Returns** `string` — A formatted card number such as `"4532 1234 5678 9012"`.

### `faker::uuid`

```kiwi
faker::uuid()
```

Returns a random UUID (version 4).

**Returns** `string` — A UUID such as `"550e8400-e29b-41d4-a716-446655440000"`.

### `faker::language`

```kiwi
faker::language()
```

Returns a random programming language name.

**Returns** `string` — A language name such as `"Rust"`.

### `faker::hashtag`

```kiwi
faker::hashtag()
```

Returns a random lorem ipsum hashtag.

**Returns** `string` — A hashtag such as `"#alias"`.

### `faker::person`

```kiwi
faker::person()
```

Generates a hashmap of fake person data.

**Returns** `hashmap` — A hashmap with `name`, `email`, `phone`, `address`, `company`, `job`, and `profession` keys.

### `faker::people`

```kiwi
faker::people(count = 10)
```

Generates a list of fake person hashmaps.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `count` | `integer` | `10` | The number of people to generate (default `10`). |

**Returns** `list` — A list of person hashmaps.
