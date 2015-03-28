context.value += " " + context.lang.l3_code() + \
                 " " + context.country.l3_code() + \
                 " " + context.guid.string() + \
                 " " + context.locale.lang.l3_code() + "/" + \
                 context.locale.country.l3_code() + \
                 " " + context.moment.rfc0822(True, True)

context.lang = el.Lang("eng")
context.country = el.Country("AFG")
context.guid.generate()
context.locale.country = el.Country("BEL")
context.locale.lang = el.Lang("spa")
context.map["xyz"] = 100
context.moment = el.Moment(el.gettimeofday())
context.nv_map["word"] = "micro"
context.seq.append("kazachok")
context.time = el.gettimeofday()

context.http_headers.append(el.net.http.Header("User-Agent", "Safari"))
context.http_params.append(el.net.http.Param("user", "John"))

context.http_accept_langs.append(\
    el.net.http.AcceptLanguage(el.Lang("ger"), "USA", 0.9))

context.http_accept_langs.append(\
    el.net.http.AcceptLanguage(el.Lang("eng"), "abc", 0.8))

context.http_search_info = el.net.http.SearchInfo("google", "apple", "mac")

context.http_request_params = \
  el.net.http.RequestParams("MSIE", "www.abc.com", 10, 20, 1000)

context.http_cookie_setter = \
  el.net.http.CookieSetter(\
    "uid",
    "1234567890",
    el.Moment(el.TimeValue(el.gettimeofday().sec() + 86400 * 365)),
    "/search",
    "xyz.com",
    True)

context.http_cookies.append(el.net.http.Cookie("UID", "17"))

logger.debug("TEST", "Some debug info here")
