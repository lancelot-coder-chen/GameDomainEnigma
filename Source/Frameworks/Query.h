﻿/*********************************************************************
 * \file   Query.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   October 2023
 *********************************************************************/
#ifndef QUERY_H
#define QUERY_H

#include <functional>
#include <memory>

namespace Enigma::Frameworks
{
    class IQuery
    {
    public:
        IQuery() {};
        IQuery(const IQuery&) = delete;
        IQuery(IQuery&&) = delete;
        virtual ~IQuery() {};
        IQuery& operator=(const IQuery&) = delete;
        IQuery& operator=(IQuery&&) = delete;
        virtual const std::type_info& typeInfo() { return typeid(*this); };  ///< 實作層的 type info
    };

    template<typename R>
    class Query : public IQuery
    {
    public:
        void setResult(const R& result) { m_result = result; }
        R getResult() const { return m_result; }

    protected:
        R m_result;
    };
    using IQueryPtr = std::shared_ptr<IQuery>;
    using QueryHandler = std::function<void(const IQueryPtr&)>;
    using QueryHandlerDelegate = void(*)(const IQueryPtr&);
}

#endif // QUERY_H