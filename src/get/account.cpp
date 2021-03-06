/*
Copyright (C) 2018 Jonathon Ogden <jeog.dev@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses.
*/

#include <iostream>

#include "../../include/_tdma_api.h"
#include "../../include/_get.h"

using std::string;
using std::vector;
using std::tie;
using std::pair;
using util::is_valid_iso8601_datetime;

namespace tdma {

class AccountGetterBaseImpl
        : public APIGetterImpl{
    string _account_id;

    virtual void
    build() = 0;

protected:
    AccountGetterBaseImpl( Credentials& creds, const string& account_id )
        :
           APIGetterImpl(creds, account_api_on_error_callback),
           _account_id(account_id)
        {
           if( account_id.empty() )
               TDMA_API_THROW(ValueException,"account_id is empty");
        }

public:
    typedef AccountGetterBase ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_ACCOUNT_INFO;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_ORDERS;

    string
    get_account_id() const
    { return _account_id; }

    void
    set_account_id(const string& account_id)
    {
        if( account_id.empty() )
            TDMA_API_THROW(ValueException,"account_id is empty");
        _account_id = account_id;
        build();
    }
};

class AccountInfoGetterImpl
        : public AccountGetterBaseImpl{
    bool _positions;
    bool _orders;

    void
    _build()
    {
        string fields;
        if(_positions){
            fields = "?fields=positions";
            if(_orders){
                fields += ",orders";
            }
        }else if(_orders){
            fields = "?fields=orders";
        }

        string url = URL_ACCOUNTS + util::url_encode(get_account_id()) + fields;
        APIGetterImpl::set_url(url);
    }

    /*virtual*/ void
    build()
    { _build(); }

public:
    typedef AccountInfoGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_ACCOUNT_INFO;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_ACCOUNT_INFO;

    AccountInfoGetterImpl( Credentials& creds,
                           const string& account_id,
                           bool positions,
                           bool orders )
        :
            AccountGetterBaseImpl(creds, account_id),
            _positions(positions),
            _orders(orders)
        {
            _build();
        }

    bool
    returns_positions() const
    { return _positions; }

    bool
    returns_orders() const
    { return _orders;}

    void
    return_positions(bool positions)
    {
        _positions = positions;
        build();
    }

    void
    return_orders(bool orders)
    {
        _orders = orders;
        build();
    }
};

class PreferencesGetterImpl
        : public AccountGetterBaseImpl{
    void
    _build()
    {
        string url = URL_ACCOUNTS + util::url_encode(get_account_id())
                     + "/preferences";
        APIGetterImpl::set_url(url);
    }

    /*virtual*/ void
    build()
    { _build(); }

public:
    typedef PreferencesGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_PREFERENCES;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_PREFERENCES;

    PreferencesGetterImpl( Credentials& creds, const string& account_id )
        :
            AccountGetterBaseImpl(creds, account_id)
        {
            _build();
        }

};


class StreamerSubscriptionKeysGetterImpl
        : public AccountGetterBaseImpl{

    void
    _build()
    {
        string params = "?accountIds=" + util::url_encode(get_account_id());
        string url = URL_BASE + "userprincipals/streamersubscriptionkeys"
                     + params;
        APIGetterImpl::set_url(url);
    }

    /*virtual*/ void
    build()
    { _build(); }

public:
    typedef StreamerSubscriptionKeysGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_SUBSCRIPTION_KEYS;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_SUBSCRIPTION_KEYS;

    StreamerSubscriptionKeysGetterImpl( Credentials& creds,
                                    const string& account_id )
        :
            AccountGetterBaseImpl(creds, account_id)
        {
            _build();
        }
};


class TransactionHistoryGetterImpl
        : public AccountGetterBaseImpl{
    string _transaction_id;
    TransactionType _transaction_type;
    string _symbol;
    string _start_date;
    string _end_date;

    void
    _build()
    {
        vector<pair<string,string>> params{{"type", to_string(_transaction_type)}};

        if( !_symbol.empty() )
            params.emplace_back("symbol", _symbol);
        if( !_start_date.empty() )
            params.emplace_back("startDate", _start_date);
        if( !_end_date.empty() )
            params.emplace_back("endDate", _end_date);

        string qstr = util::build_encoded_query_str(params);
        string url = URL_ACCOUNTS + util::url_encode(get_account_id())
                     + "/transactions?" + qstr;
        APIGetterImpl::set_url(url);
    }

    /*virtual*/ void
    build()
    { _build(); }

public:
    typedef TransactionHistoryGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_TRANSACTION_HISTORY;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_TRANSACTION_HISTORY;

    TransactionHistoryGetterImpl( Credentials& creds,
                                  const string& account_id,
                                  TransactionType transaction_type,
                                  const string& symbol,
                                  const string& start_date,
                                  const string& end_date )
        :
            AccountGetterBaseImpl(creds, account_id),
            _transaction_type(transaction_type),
            _symbol( util::toupper(symbol) ),
            _start_date(start_date),
            _end_date(end_date)
        {
            if( !start_date.empty() && !is_valid_iso8601_datetime(start_date) ){
                TDMA_API_THROW( ValueException,
                                "invalid ISO-8601 date: " + start_date );
            }

            if( !end_date.empty() && !is_valid_iso8601_datetime(end_date) ){
                TDMA_API_THROW( ValueException,
                                "invalid ISO-8601 date: " + end_date );
            }

            _build();
        }

    TransactionType
    get_transaction_type() const
    { return _transaction_type; }

    string
    get_symbol() const
    { return _symbol; }

    string
    get_start_date() const
    { return _start_date; }

    string
    get_end_date() const
    { return _end_date; }

    void
    set_transaction_type(TransactionType transaction_type)
    {
        _transaction_type = transaction_type;
        build();
    }

    void
    set_symbol(const string& symbol)
    {
        _symbol = util::toupper(symbol);
        build();
    }

    void
    set_start_date(const string& start_date)
    {
        if( !start_date.empty() && !is_valid_iso8601_datetime(start_date) ){
            TDMA_API_THROW( ValueException,
                            "invalid ISO-8601 date: " + start_date );
        }
        _start_date = start_date;
        build();
    }

    void
    set_end_date(const string& end_date)
    {
        if( !end_date.empty() && !is_valid_iso8601_datetime(end_date) ){
            TDMA_API_THROW( ValueException,
                            "invalid ISO-8601 date: " + end_date );
        }
        _end_date = end_date;
        build();
    }

};


class IndividualTransactionHistoryGetterImpl
        : public AccountGetterBaseImpl {
    string _transaction_id;

    void
    _build()
    {
        string url = URL_ACCOUNTS + util::url_encode(get_account_id())
                     + "/transactions/" + util::url_encode(_transaction_id);
        APIGetterImpl::set_url(url);
    }

    /*virtual*/ void
    build()
    { _build(); }

public:
    typedef IndividualTransactionHistoryGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_IND_TRANSACTION_HISTORY;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_IND_TRANSACTION_HISTORY;

    IndividualTransactionHistoryGetterImpl( Credentials& creds,
                                            const string& account_id,
                                            const string& transaction_id )
        :
            AccountGetterBaseImpl(creds, account_id),
            _transaction_id(transaction_id)
        {
            if( transaction_id.empty() )
                TDMA_API_THROW(ValueException,"transaction id is empty");

            _build();
        }

    string
    get_transaction_id() const
    { return _transaction_id; }

    void
    set_transaction_id(const string& transaction_id)
    {
         if( transaction_id.empty() )
             TDMA_API_THROW(ValueException,"transaction id is empty");

         _transaction_id = transaction_id;
         build();
     }

};


class UserPrincipalsGetterImpl
        : public APIGetterImpl{
    bool _streamer_subscription_keys;
    bool _streamer_connection_info;
    bool _preferences;
    bool _surrogate_ids;

    void
    _build()
    {
        vector<string> fields;
        if(_streamer_subscription_keys)
            fields.emplace_back("streamerSubscriptionKeys");
        if(_streamer_connection_info)
            fields.emplace_back("streamerConnectionInfo");
        if(_preferences)
            fields.emplace_back("preferences");
        if(_surrogate_ids)
            fields.emplace_back("surrogateIds");

        string fields_str;
        if( !fields.empty() ){
            fields_str = "?fields=" + util::join(fields, ',');
        }

        string url = URL_BASE + "userprincipals" + fields_str;
        APIGetterImpl::set_url(url);
    }

    /*virtual*/ void
    build()
    { _build(); }

public:
    typedef UserPrincipalsGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_USER_PRINCIPALS;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_USER_PRINCIPALS;

    UserPrincipalsGetterImpl( Credentials& creds,
                              bool streamer_subscription_keys,
                              bool streamer_connection_info,
                              bool preferences,
                              bool surrogate_ids )
        :
            APIGetterImpl(creds, account_api_on_error_callback),
            _streamer_subscription_keys(streamer_subscription_keys),
            _streamer_connection_info(streamer_connection_info),
            _preferences(preferences),
            _surrogate_ids(surrogate_ids)
        {
            _build();
        }

    bool
    returns_streamer_subscription_keys() const
    { return _streamer_subscription_keys; }

    bool
    returns_streamer_connection_info() const
    { return _streamer_connection_info; }

    bool
    returns_preferences() const
    { return _preferences; }

    bool
    returns_surrogate_ids() const
    { return _surrogate_ids; }

    void
    return_streamer_subscription_keys(bool streamer_subscription_keys)
    {
        _streamer_subscription_keys = streamer_subscription_keys;
        build();
    }

    void
    return_streamer_connection_info(bool streamer_connection_info)
    {
        _streamer_connection_info = streamer_connection_info;
        build();
    }

    void
    return_preferences(bool preferences)
    {
        _preferences = preferences;
        build();
    }

    void
    return_surrogate_ids(bool surrogate_ids)
    {
        _surrogate_ids = surrogate_ids;
        build();
    }
};

json
get_user_principals_for_streaming(Credentials& creds)
{
    string s = UserPrincipalsGetterImpl(creds,true,true,false,false).get();
    return s.empty() ? json() : json::parse(s);
}


class OrderGetterImpl
        : public AccountGetterBaseImpl {
    string _order_id;

    void
    _build()
    {
        string url = URL_ACCOUNTS + util::url_encode(get_account_id())
                    + "/orders/" + util::url_encode(_order_id);
        APIGetterImpl::set_url(url);
    }

    virtual void
    build()
    { _build(); }

public:
    typedef OrderGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_ORDER;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_ORDER;

    OrderGetterImpl( Credentials& creds,
                     const string& account_id,
                     const string& order_id )
        :
            AccountGetterBaseImpl(creds, account_id),
            _order_id(order_id)
        {
            if( order_id.empty() )
                TDMA_API_THROW(ValueException,"empty order ID");

            _build();
        }

    string
    get_order_id() const
    { return _order_id; }

    void
    set_order_id(const string& order_id)
    {
        if( order_id.empty() )
            TDMA_API_THROW(ValueException,"empty order ID");

        _order_id = order_id;
        build();
    }
};


class OrdersGetterImpl
        : public AccountGetterBaseImpl {
    unsigned int _nmax_results;
    string _from_entered_time;
    string _to_entered_time;
    OrderStatusType _order_status_type;

    void
    _build()
    {
        vector<pair<string,string>> params{
            {"maxResults", std::to_string(_nmax_results)},
            {"fromEnteredTime", _from_entered_time},
            {"toEnteredTime", _to_entered_time},
            {"status", to_string(_order_status_type)}
        };

        string qstr = util::build_encoded_query_str(params);
        string url = URL_ACCOUNTS + util::url_encode(get_account_id())
                    + "/orders?" + qstr;

        APIGetterImpl::set_url(url);
    }

    virtual void
    build()
    { _build(); }

public:
    typedef OrdersGetter ProxyType;
    static const int TYPE_ID_LOW = TYPE_ID_GETTER_ORDERS;
    static const int TYPE_ID_HIGH = TYPE_ID_GETTER_ORDERS;

    OrdersGetterImpl( Credentials& creds,
                      const string& account_id,
                      unsigned int nmax_results,
                      const string& from_entered_time,
                      const string& to_entered_time,
                      OrderStatusType order_status_type )
        :
            AccountGetterBaseImpl(creds, account_id),
            _nmax_results(nmax_results),
            _from_entered_time(from_entered_time),
            _to_entered_time(to_entered_time),
            _order_status_type(order_status_type)
        {
            if( nmax_results < 1 ){
                TDMA_API_THROW(ValueException,"nmax_results < 1");
            }
            if( !is_valid_iso8601_datetime(from_entered_time) ){
                TDMA_API_THROW( ValueException,
                    "invalid ISO-8601 date/time: " + from_entered_time );
            }
            if( !is_valid_iso8601_datetime(to_entered_time) ){
                TDMA_API_THROW( ValueException,
                    "invalid ISO-8601 date/time: " + to_entered_time );
            }
            _build();
        }

    unsigned int
    get_nmax_results() const
    { return _nmax_results; }

    string
    get_from_entered_time() const
    { return _from_entered_time; }

    string
    get_to_entered_time() const
    { return _to_entered_time; }

    OrderStatusType
    get_order_status_type() const
    { return _order_status_type; }

    void
    set_nmax_results(unsigned int nmax_results)
    {
        if( nmax_results < 1 )
            TDMA_API_THROW(ValueException,"nmax_results < 1");

        _nmax_results = nmax_results;
        build();
    }

    void
    set_from_entered_time(const string& from_entered_time)
    {
        if( !is_valid_iso8601_datetime(from_entered_time) ){
            TDMA_API_THROW( ValueException,
                            "invalid ISO-8601 date/time: " + from_entered_time );
        }

        _from_entered_time = from_entered_time;
        build();
    }

    void
    set_to_entered_time(const string& to_entered_time)
    {
        if( !is_valid_iso8601_datetime(to_entered_time) ){
            TDMA_API_THROW( ValueException,
                            "invalid ISO-8601 date/time: " + to_entered_time );
        }

        _to_entered_time = to_entered_time;
        build();
    }

    void
    set_order_status_type(OrderStatusType order_status_type)
    {
        _order_status_type = order_status_type;
        build();
    }
};

} /* tdma */

using namespace tdma;

int
AccountGetterBase_GetAccountId_ABI( Getter_C *pgetter,
                                    char **buf,
                                    size_t *n,
                                    int allow_exceptions )
{
    return ImplAccessor<char**>::template get<AccountGetterBaseImpl>(
        pgetter, &AccountGetterBaseImpl::get_account_id, buf, n, allow_exceptions
        );
}

int
AccountGetterBase_SetAccountId_ABI( Getter_C *pgetter,
                                    const char *account_id,
                                    int allow_exceptions )
{
    return ImplAccessor<char**>::template set<AccountGetterBaseImpl>(
        pgetter, &AccountGetterBaseImpl::set_account_id, account_id,
        allow_exceptions
        );
}

int
AccountInfoGetter_Create_ABI( struct Credentials *pcreds,
                              const char* account_id,
                              int positions,
                              int orders,
                              AccountInfoGetter_C *pgetter,
                              int allow_exceptions )
{
    using ImplTy = AccountInfoGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_PTR_KILL_PROXY( account_id, "account_id", allow_exceptions,
                               pgetter );

    static auto meth = +[]( Credentials *c, const char* s, int p, int o ){
        return new ImplTy(*c, s, static_cast<bool>(p), static_cast<bool>(o) );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI( allow_exceptions, meth, pcreds, account_id,
                                     positions, orders);
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    assert( ImplTy::TYPE_ID_LOW == ImplTy::TYPE_ID_HIGH );
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}

int
AccountInfoGetter_Destroy_ABI( AccountInfoGetter_C *pgetter,
                               int allow_exceptions )
{
    return destroy_proxy<AccountInfoGetterImpl>(pgetter, allow_exceptions);
}

int
AccountInfoGetter_ReturnsPositions_ABI( AccountInfoGetter_C *pgetter,
                                        int *returns_positions,
                                        int allow_exceptions )
{
    return ImplAccessor<int>::template
        get<AccountInfoGetterImpl, bool>(
            pgetter, &AccountInfoGetterImpl::returns_positions,
            returns_positions, "returns_positions", allow_exceptions
        );
}

int
AccountInfoGetter_ReturnPositions_ABI( AccountInfoGetter_C *pgetter,
                                       int return_positions,
                                       int allow_exceptions )
{
    return ImplAccessor<int>::template
        set<AccountInfoGetterImpl, bool>(
            pgetter, &AccountInfoGetterImpl::return_positions,
            return_positions, allow_exceptions
        );
}

int
AccountInfoGetter_ReturnsOrders_ABI( AccountInfoGetter_C *pgetter,
                                     int *returns_orders,
                                     int allow_exceptions )
{
    return ImplAccessor<int>::template
        get<AccountInfoGetterImpl, bool>(
            pgetter, &AccountInfoGetterImpl::returns_orders,
            returns_orders, "returns_orders", allow_exceptions
        );
}

int
AccountInfoGetter_ReturnOrders_ABI( AccountInfoGetter_C *pgetter,
                                    int return_orders,
                                    int allow_exceptions )
{
    return ImplAccessor<int>::template
        set<AccountInfoGetterImpl, bool>(
            pgetter, &AccountInfoGetterImpl::return_orders,
            return_orders, allow_exceptions
        );
}

int
PreferencesGetter_Create_ABI( struct Credentials *pcreds,
                              const char* account_id,
                              PreferencesGetter_C *pgetter,
                              int allow_exceptions )
{
    using ImplTy = PreferencesGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_PTR_KILL_PROXY( account_id, "account_id", allow_exceptions,
                               pgetter );

    static auto meth = +[]( Credentials *c, const char* s ){
        return new ImplTy( *c, s );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI(allow_exceptions, meth, pcreds, account_id);
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    assert( ImplTy::TYPE_ID_LOW == ImplTy::TYPE_ID_HIGH );
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}

int
PreferencesGetter_Destroy_ABI( PreferencesGetter_C *pgetter,
                               int allow_exceptions )
{ return destroy_proxy<PreferencesGetterImpl>(pgetter, allow_exceptions); }


int
StreamerSubscriptionKeysGetter_Create_ABI(
     struct Credentials *pcreds,
     const char* account_id,
     StreamerSubscriptionKeysGetter_C *pgetter,
     int allow_exceptions
     )
{
    using ImplTy = StreamerSubscriptionKeysGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_PTR_KILL_PROXY( account_id, "account_id", allow_exceptions,
                               pgetter );

    static auto meth = +[]( Credentials *c, const char* s ){
        return new ImplTy( *c, s );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI(allow_exceptions, meth, pcreds, account_id);
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    assert( ImplTy::TYPE_ID_LOW == ImplTy::TYPE_ID_HIGH );
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}

int
StreamerSubscriptionKeysGetter_Destroy_ABI(
    StreamerSubscriptionKeysGetter_C *pgetter,
    int allow_exceptions
    )
{
    return destroy_proxy<StreamerSubscriptionKeysGetterImpl>(
        pgetter, allow_exceptions
    );
}


int
TransactionHistoryGetter_Create_ABI( struct Credentials *pcreds,
                                     const char* account_id,
                                     int transaction_type,
                                     const char* symbol,
                                     const char* start_date,
                                     const char* end_date,
                                     TransactionHistoryGetter_C *pgetter,
                                     int allow_exceptions )
{
    using ImplTy = TransactionHistoryGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_ENUM_KILL_PROXY( TransactionType, transaction_type,
                               allow_exceptions, pgetter );

    CHECK_PTR_KILL_PROXY(account_id, "account_id", allow_exceptions, pgetter);

    static auto meth = +[]( Credentials *c, const char* s, int t,
                            const char* sym, const char* sd, const char* ed ){
        return new ImplTy( *c, s, static_cast<TransactionType>(t),
                           (sym ? sym : ""), (sd ? sd : ""), (ed ? ed : "") );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI( allow_exceptions, meth, pcreds, account_id,
                                     transaction_type, symbol, start_date,
                                     end_date);
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    assert( ImplTy::TYPE_ID_LOW == ImplTy::TYPE_ID_HIGH );
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}

int
TransactionHistoryGetter_Destroy_ABI( TransactionHistoryGetter_C *pgetter,
                                      int allow_exceptions )
{ return destroy_proxy<TransactionHistoryGetterImpl>(pgetter, allow_exceptions); }

int
TransactionHistoryGetter_GetTransactionType_ABI(
    TransactionHistoryGetter_C *pgetter,
    int *transaction_type,
    int allow_exceptions )
{
    return ImplAccessor<int>::template
        get<TransactionHistoryGetterImpl, TransactionType>(
            pgetter, &TransactionHistoryGetterImpl::get_transaction_type,
            transaction_type, "transaction_type", allow_exceptions
        );
}

int
TransactionHistoryGetter_SetTransactionType_ABI(
    TransactionHistoryGetter_C *pgetter,
    int transaction_type,
    int allow_exceptions )
{
    CHECK_ENUM(TransactionType, transaction_type, allow_exceptions);

    return ImplAccessor<int>::template
        set<TransactionHistoryGetterImpl, TransactionType>(
            pgetter, &TransactionHistoryGetterImpl::set_transaction_type,
            transaction_type, allow_exceptions
        );
}

int
TransactionHistoryGetter_GetSymbol_ABI(
    TransactionHistoryGetter_C *pgetter,
    char **buf,
    size_t *n,
    int allow_exceptions )
{
    return ImplAccessor<char**>::template
        get<TransactionHistoryGetterImpl>(
            pgetter, &TransactionHistoryGetterImpl::get_symbol,
            buf, n, allow_exceptions
        );
}

int
TransactionHistoryGetter_SetSymbol_ABI(
    TransactionHistoryGetter_C *pgetter,
    const char* symbol,
    int allow_exceptions )
{
    return ImplAccessor<char**>::template
        set<TransactionHistoryGetterImpl>(
            pgetter, &TransactionHistoryGetterImpl::set_symbol,
            symbol, allow_exceptions
        );
}


int
TransactionHistoryGetter_GetStartDate_ABI(
    TransactionHistoryGetter_C *pgetter,
    char **buf,
    size_t *n,
    int allow_exceptions )
{
    return ImplAccessor<char**>::template
        get<TransactionHistoryGetterImpl>(
            pgetter, &TransactionHistoryGetterImpl::get_start_date,
            buf, n, allow_exceptions
        );
}

int
TransactionHistoryGetter_SetStartDate_ABI(
    TransactionHistoryGetter_C *pgetter,
    const char* start_date,
    int allow_exceptions )
{
    return ImplAccessor<char**>::template
        set<TransactionHistoryGetterImpl>(
            pgetter, &TransactionHistoryGetterImpl::set_start_date,
            start_date, allow_exceptions
        );
}


int
TransactionHistoryGetter_GetEndDate_ABI(
    TransactionHistoryGetter_C *pgetter,
    char **buf,
    size_t *n,
    int allow_exceptions )
{
    return ImplAccessor<char**>::template
        get<TransactionHistoryGetterImpl>(
            pgetter, &TransactionHistoryGetterImpl::get_end_date,
            buf, n, allow_exceptions
        );
}

int
TransactionHistoryGetter_SetEndDate_ABI(
    TransactionHistoryGetter_C *pgetter,
    const char* end_date,
    int allow_exceptions )
{
    return ImplAccessor<char**>::template
        set<TransactionHistoryGetterImpl>(
            pgetter, &TransactionHistoryGetterImpl::set_end_date,
            end_date, allow_exceptions
        );
}

int
IndividualTransactionHistoryGetter_Create_ABI(
    struct Credentials *pcreds,
    const char* account_id,
    const char* transaction_id,
    IndividualTransactionHistoryGetter_C *pgetter,
    int allow_exceptions )
{
    using ImplTy = IndividualTransactionHistoryGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_PTR_KILL_PROXY( account_id, "account_id", allow_exceptions,
                               pgetter );

    CHECK_PTR_KILL_PROXY( transaction_id, "transaction_id",
                               allow_exceptions, pgetter );

    static auto meth = +[]( Credentials *c, const char* s, const char* t ){
        return new ImplTy( *c, s, t );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI( allow_exceptions, meth, pcreds, account_id,
                                     transaction_id );
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    assert( ImplTy::TYPE_ID_LOW == ImplTy::TYPE_ID_HIGH );
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}

int
IndividualTransactionHistoryGetter_Destroy_ABI(
    IndividualTransactionHistoryGetter_C *pgetter,
    int allow_exceptions
    )
{
    return destroy_proxy<IndividualTransactionHistoryGetterImpl>(
        pgetter, allow_exceptions
        );
}

int
IndividualTransactionHistoryGetter_GetTransactionId_ABI(
    IndividualTransactionHistoryGetter_C *pgetter,
    char **buf,
    size_t *n,
    int allow_exceptions
    )
{
    return ImplAccessor<char**>::template
        get<IndividualTransactionHistoryGetterImpl>(
            pgetter, &IndividualTransactionHistoryGetterImpl::get_transaction_id,
            buf, n, allow_exceptions
        );
}

int
IndividualTransactionHistoryGetter_SetTransactionId_ABI(
    IndividualTransactionHistoryGetter_C *pgetter,
    const char* transaction_id,
    int allow_exceptions
    )
{
    return ImplAccessor<char**>::template
        set<IndividualTransactionHistoryGetterImpl>(
            pgetter, &IndividualTransactionHistoryGetterImpl::set_transaction_id,
            transaction_id, allow_exceptions
        );
}

int
UserPrincipalsGetter_Create_ABI( struct Credentials *pcreds,
                                 int streamer_subscription_keys,
                                 int streamer_connection_info,
                                 int preferences,
                                 int surrogate_ids,
                                 UserPrincipalsGetter_C *pgetter,
                                 int allow_exceptions )
{
    using ImplTy = UserPrincipalsGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    static auto meth = +[]( Credentials *c, int k, int i, int p, int si ){
        return new ImplTy( *c, k, i, p, si );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI( allow_exceptions, meth, pcreds,
                                     streamer_subscription_keys,
                                     streamer_connection_info, preferences,
                                     surrogate_ids );
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    assert( ImplTy::TYPE_ID_LOW == ImplTy::TYPE_ID_HIGH );
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}


int
UserPrincipalsGetter_Destroy_ABI( UserPrincipalsGetter_C *pgetter,
                                  int allow_exceptions )
{ return destroy_proxy<UserPrincipalsGetterImpl>(pgetter, allow_exceptions); }

int
UserPrincipalsGetter_ReturnsSubscriptionKeys_ABI(
    UserPrincipalsGetter_C *pgetter,
    int *returns_subscription_keys,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        get<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::returns_streamer_subscription_keys,
            returns_subscription_keys, "returns_subscription_keys",
            allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnSubscriptionKeys_ABI(
    UserPrincipalsGetter_C *pgetter,
    int return_subscription_keys,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        set<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::return_streamer_subscription_keys,
            return_subscription_keys, allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnsConnectionInfo_ABI(
    UserPrincipalsGetter_C *pgetter,
    int *returns_connection_info,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        get<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::returns_streamer_connection_info,
            returns_connection_info, "returns_connection_info", allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnConnectionInfo_ABI(
    UserPrincipalsGetter_C *pgetter,
    int return_connection_info,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        set<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::return_streamer_connection_info,
            return_connection_info, allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnsPreferences_ABI(
    UserPrincipalsGetter_C *pgetter,
    int *returns_preferences,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        get<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::returns_preferences,
            returns_preferences, "returns_preferences", allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnPreferences_ABI(
    UserPrincipalsGetter_C *pgetter,
    int return_preferences,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        set<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::return_preferences,
            return_preferences, allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnsSurrogateIds_ABI(
    UserPrincipalsGetter_C *pgetter,
    int *returns_surrogate_ids,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        get<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::returns_surrogate_ids,
            returns_surrogate_ids, "returns_surrogate_ids", allow_exceptions
        );
}

int
UserPrincipalsGetter_ReturnSurrogateIds_ABI(
    UserPrincipalsGetter_C *pgetter,
    int return_surrogate_ids,
    int allow_exceptions
    )
{
    return ImplAccessor<int>::template
        set<UserPrincipalsGetterImpl, bool>(
            pgetter, &UserPrincipalsGetterImpl::return_surrogate_ids,
            return_surrogate_ids, allow_exceptions
        );
}

int
OrderGetter_Create_ABI( Credentials *pcreds,
                        const char* account_id,
                        const char* order_id,
                        OrderGetter_C *pgetter,
                        int allow_exceptions )
{
    using ImplTy = OrderGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_PTR_KILL_PROXY(account_id, "account id", allow_exceptions, pgetter);
    CHECK_PTR_KILL_PROXY(order_id, "order id", allow_exceptions, pgetter);

    static auto meth = +[](Credentials *c, const char* aid, const char* oid){
        return new ImplTy(*c, aid, oid);
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI(allow_exceptions, meth, pcreds,
                                    account_id, order_id);
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}


int
OrderGetter_Destroy_ABI(OrderGetter_C *pgetter, int allow_exceptions)
{
    return destroy_proxy<OrderGetterImpl>(pgetter, allow_exceptions);
}


int
OrderGetter_GetOrderId_ABI( OrderGetter_C *pgetter,
                            char **buf,
                            size_t *n,
                            int allow_exceptions)
{
    return ImplAccessor<char**>::template get<OrderGetterImpl>(
        pgetter, &OrderGetterImpl::get_order_id, buf, n, allow_exceptions
        );
}


int
OrderGetter_SetOrderId_ABI( OrderGetter_C *pgetter,
                            const char *order_id,
                            int allow_exceptions )
{
    return ImplAccessor<char**>::template set<OrderGetterImpl>(
        pgetter, &OrderGetterImpl::set_order_id, order_id, allow_exceptions
        );
}


int
OrdersGetter_Create_ABI( struct Credentials *pcreds,
                         const char* account_id,
                         unsigned int nmax_results,
                         const char* from_entered_time,
                         const char* to_entered_time,
                         int order_status_type,
                         OrdersGetter_C *pgetter,
                         int allow_exceptions )
{
    using ImplTy = OrdersGetterImpl;

    int err = getter_is_creatable<ImplTy>(pcreds, pgetter, allow_exceptions);
    if( err )
        return err;

    CHECK_PTR_KILL_PROXY(account_id, "account id", allow_exceptions, pgetter);
    CHECK_PTR_KILL_PROXY(from_entered_time, "from entered time",
                         allow_exceptions, pgetter);
    CHECK_PTR_KILL_PROXY(to_entered_time, "to entered time",
                         allow_exceptions, pgetter);
    CHECK_ENUM_KILL_PROXY(OrderStatusType, order_status_type,
                          allow_exceptions, pgetter);

    static auto meth = +[](Credentials *c, const char* aid, unsigned int n,
                           const char* from, const char* to, int os){
        return new ImplTy( *c, aid, n, from, to,
                           static_cast<OrderStatusType>(os) );
    };

    ImplTy *obj;
    tie(obj, err) = CallImplFromABI(allow_exceptions, meth, pcreds, account_id,
                                    nmax_results, from_entered_time,
                                    to_entered_time, order_status_type);
    if( err ){
        kill_proxy(pgetter);
        return err;
    }

    pgetter->obj = reinterpret_cast<void*>(obj);
    pgetter->type_id = ImplTy::TYPE_ID_LOW;
    return 0;
}

int
OrdersGetter_Destroy_ABI(OrdersGetter_C *pgetter, int allow_exceptions)
{
    return destroy_proxy<OrdersGetterImpl>(pgetter, allow_exceptions);
}

int
OrdersGetter_GetNMaxResults_ABI( OrdersGetter_C *pgetter,
                                 unsigned int *nmax_results,
                                 int allow_exceptions)
{
    return ImplAccessor<unsigned int>::template
        get<OrdersGetterImpl>(
            pgetter, &OrdersGetterImpl::get_nmax_results,
            nmax_results, "nmax_results", allow_exceptions
        );
}

int
OrdersGetter_SetNMaxResults_ABI( OrdersGetter_C *pgetter,
                                 unsigned int nmax_results,
                                 int allow_exceptions )
{
    return ImplAccessor<unsigned int>::template
        set<OrdersGetterImpl>(
            pgetter, &OrdersGetterImpl::set_nmax_results,
            nmax_results, allow_exceptions
        );
}

int
OrdersGetter_GetFromEnteredTime_ABI( OrdersGetter_C *pgetter,
                                     char** buf,
                                     size_t *n,
                                     int allow_exceptions )
{
    return ImplAccessor<char**>::template
        get<OrdersGetterImpl>(
            pgetter, &OrdersGetterImpl::get_from_entered_time, buf, n,
            allow_exceptions
        );
}

int
OrdersGetter_SetFromEnteredTime_ABI( OrdersGetter_C *pgetter,
                                     const char* from_entered_time,
                                     int allow_exceptions )
{
    return ImplAccessor<char**>::template
        set<OrdersGetterImpl>(
            pgetter, &OrdersGetterImpl::set_from_entered_time,
            from_entered_time, allow_exceptions
        );
}

int
OrdersGetter_GetToEnteredTime_ABI( OrdersGetter_C *pgetter,
                                   char** buf,
                                   size_t *n,
                                   int allow_exceptions )
{
    return ImplAccessor<char**>::template
        get<OrdersGetterImpl>(
            pgetter, &OrdersGetterImpl::get_to_entered_time, buf, n,
            allow_exceptions
        );
}

int
OrdersGetter_SetToEnteredTime_ABI( OrdersGetter_C *pgetter,
                                   const char* to_entered_time,
                                   int allow_exceptions )
{
    return ImplAccessor<char**>::template
        set<OrdersGetterImpl>(
            pgetter, &OrdersGetterImpl::set_to_entered_time,
            to_entered_time, allow_exceptions
        );
}

int
OrdersGetter_GetOrderStatusType_ABI( OrdersGetter_C *pgetter,
                                     int *order_status_type,
                                     int allow_exceptions )
{
    return ImplAccessor<int>::template
        get<OrdersGetterImpl, OrderStatusType>(
            pgetter, &OrdersGetterImpl::get_order_status_type,
            order_status_type, "order_status_type", allow_exceptions
        );
}

int
OrdersGetter_SetOrderStatusType_ABI( OrdersGetter_C *pgetter,
                                     int order_status_type,
                                     int allow_exceptions )
{
    CHECK_ENUM(OrderStatusType, order_status_type, allow_exceptions);

    return ImplAccessor<int>::template
        set<OrdersGetterImpl, OrderStatusType>(
            pgetter, &OrdersGetterImpl::set_order_status_type,
            order_status_type, allow_exceptions
        );
}

