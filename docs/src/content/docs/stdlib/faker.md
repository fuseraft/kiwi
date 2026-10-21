---
title: faker
description: Generate realistic fake data for tests, demos and seed files.
---

The `faker` package generates random but realistic-looking data: names, emails, addresses, companies, text, numbers and more. Every call returns a new random value.

```kiwi nocheck
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

<!-- api: faker -->
