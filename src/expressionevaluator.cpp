#include "expressionevaluator.h"

#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QtGlobal>

namespace {
bool isIdentifierStart(QChar ch)
{
    return ch.isLetter() || ch == QLatin1Char('_');
}

bool isIdentifierPart(QChar ch)
{
    return ch.isLetterOrNumber() || ch == QLatin1Char('_');
}
}

bool ExpressionEvaluator::ReadInput(const QString &expression, QString *errorMessage)
{
    if (errorMessage) {
        errorMessage->clear();
    }

    if (!tokenize(expression, errorMessage)) {
        return false;
    }

    m_position = 0;
    auto rootNode = parseExpression(errorMessage);
    if (!rootNode) {
        return false;
    }

    if (!expect(TokenType::End, nullptr, errorMessage)) {
        return false;
    }

    m_root = std::move(rootNode);
    return true;
}

bool ExpressionEvaluator::Assign(const QString &assignments, QString *errorMessage)
{
    if (errorMessage) {
        errorMessage->clear();
    }

    m_variables.clear();

    const QStringList lines = assignments.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (!parseAssignmentLine(line, errorMessage)) {
            return false;
        }
    }

    return true;
}

bool ExpressionEvaluator::Evaluate(double *result, QString *errorMessage) const
{
    QString internalError;
    QString *message = errorMessage ? errorMessage : &internalError;
    message->clear();

    if (!m_root) {
        *message = QObject::tr("请先构建表达式树。");
        return false;
    }

    const double value = evaluateNode(m_root.get(), message);
    if (!message->isEmpty()) {
        return false;
    }

    if (result) {
        *result = value;
    }
    return true;
}

const ExpressionEvaluator::Node *ExpressionEvaluator::root() const
{
    return m_root.get();
}

bool ExpressionEvaluator::tokenize(const QString &expression, QString *errorMessage)
{
    m_tokens.clear();
    m_position = 0;

    const int length = expression.length();
    int index = 0;
    while (index < length) {
        const QChar ch = expression.at(index);
        if (ch.isSpace()) {
            ++index;
            continue;
        }

        if (ch.isDigit() || ch == QLatin1Char('.')) {
            const int start = index;
            bool dotEncountered = (ch == QLatin1Char('.'));
            ++index;
            while (index < length) {
                const QChar nextChar = expression.at(index);
                if (nextChar.isDigit()) {
                    ++index;
                } else if (nextChar == QLatin1Char('.') && !dotEncountered) {
                    dotEncountered = true;
                    ++index;
                } else {
                    break;
                }
            }
            const QString numberText = expression.mid(start, index - start);
            bool ok = false;
            const double numberValue = numberText.toDouble(&ok);
            if (!ok) {
                if (errorMessage) {
                    *errorMessage = QObject::tr("无法解析数字：%1").arg(numberText);
                }
                return false;
            }
            Token token;
            token.type = TokenType::Number;
            token.text = numberText;
            token.numberValue = numberValue;
            m_tokens.append(token);
            continue;
        }

        if (isIdentifierStart(ch)) {
            const int start = index;
            ++index;
            while (index < length && isIdentifierPart(expression.at(index))) {
                ++index;
            }
            Token token;
            token.type = TokenType::Identifier;
            token.text = expression.mid(start, index - start);
            m_tokens.append(token);
            continue;
        }

        if (ch == QLatin1Char('+') || ch == QLatin1Char('-') || ch == QLatin1Char('*') || ch == QLatin1Char('/')) {
            Token token;
            token.type = TokenType::Operator;
            token.text = QString(ch);
            m_tokens.append(token);
            ++index;
            continue;
        }

        if (ch == QLatin1Char('(')) {
            Token token;
            token.type = TokenType::LeftParen;
            token.text = QString(ch);
            m_tokens.append(token);
            ++index;
            continue;
        }

        if (ch == QLatin1Char(')')) {
            Token token;
            token.type = TokenType::RightParen;
            token.text = QString(ch);
            m_tokens.append(token);
            ++index;
            continue;
        }

        if (errorMessage) {
            *errorMessage = QObject::tr("无法识别的字符：%1").arg(ch);
        }
        return false;
    }

    Token endToken;
    endToken.type = TokenType::End;
    m_tokens.append(endToken);
    return true;
}

const ExpressionEvaluator::Token &ExpressionEvaluator::currentToken() const
{
    static const Token kEndToken;
    if (m_tokens.isEmpty()) {
        return kEndToken;
    }
    const int clamped = qBound(0, m_position, m_tokens.size() - 1);
    return m_tokens.at(clamped);
}

bool ExpressionEvaluator::match(TokenType type, Token *out)
{
    if (currentToken().type != type) {
        return false;
    }

    if (out) {
        *out = currentToken();
    }
    ++m_position;
    return true;
}

bool ExpressionEvaluator::expect(TokenType type, Token *out, QString *errorMessage)
{
    if (match(type, out)) {
        return true;
    }

    if (errorMessage) {
        switch (type) {
        case TokenType::RightParen:
            *errorMessage = QObject::tr("缺少右括号。");
            break;
        case TokenType::End:
            *errorMessage = QObject::tr("表达式在意外的位置结束。");
            break;
        default:
            *errorMessage = QObject::tr("遇到意外的符号：%1").arg(currentToken().text);
            break;
        }
    }
    return false;
}

QChar ExpressionEvaluator::ReadOperator(const Token &token) const
{
    if (token.type != TokenType::Operator || token.text.isEmpty()) {
        return QChar();
    }
    return token.text.at(0);
}

std::unique_ptr<ExpressionEvaluator::Node> ExpressionEvaluator::MakeExp(std::unique_ptr<Node> left, QChar op, std::unique_ptr<Node> right)
{
    auto node = std::make_unique<Node>();
    node->type = Node::Type::Operator;
    node->text = QString(op);
    node->left = std::move(left);
    node->right = std::move(right);
    return node;
}

std::unique_ptr<ExpressionEvaluator::Node> ExpressionEvaluator::parseExpression(QString *errorMessage)
{
    auto left = parseTerm(errorMessage);
    if (!left) {
        return nullptr;
    }

    while (currentToken().type == TokenType::Operator) {
        const QString tokenText = currentToken().text;
        if (tokenText != QStringLiteral("+") && tokenText != QStringLiteral("-")) {
            break;
        }

        Token opToken;
        match(TokenType::Operator, &opToken);
        const QChar op = ReadOperator(opToken);
        auto right = parseTerm(errorMessage);
        if (!right) {
            return nullptr;
        }
        left = MakeExp(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionEvaluator::Node> ExpressionEvaluator::parseTerm(QString *errorMessage)
{
    auto left = parseFactor(errorMessage);
    if (!left) {
        return nullptr;
    }

    while (currentToken().type == TokenType::Operator) {
        const QString tokenText = currentToken().text;
        if (tokenText != QStringLiteral("*") && tokenText != QStringLiteral("/")) {
            break;
        }

        Token opToken;
        match(TokenType::Operator, &opToken);
        const QChar op = ReadOperator(opToken);
        auto right = parseFactor(errorMessage);
        if (!right) {
            return nullptr;
        }
        left = MakeExp(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ExpressionEvaluator::Node> ExpressionEvaluator::parseFactor(QString *errorMessage)
{
    if (currentToken().type == TokenType::Operator) {
        const QString text = currentToken().text;
        if (text == QStringLiteral("+") || text == QStringLiteral("-")) {
            Token opToken;
            match(TokenType::Operator, &opToken);
            auto factor = parseFactor(errorMessage);
            if (!factor) {
                return nullptr;
            }
            if (text == QStringLiteral("-")) {
                auto zero = std::make_unique<Node>();
                zero->type = Node::Type::Number;
                zero->text = QStringLiteral("0");
                zero->value = 0.0;
                return MakeExp(std::move(zero), QLatin1Char('-'), std::move(factor));
            }
            return factor;
        }
    }

    Token token;
    if (match(TokenType::Number, &token)) {
        auto node = std::make_unique<Node>();
        node->type = Node::Type::Number;
        node->text = token.text;
        node->value = token.numberValue;
        return node;
    }

    if (match(TokenType::Identifier, &token)) {
        auto node = std::make_unique<Node>();
        node->type = Node::Type::Variable;
        node->text = token.text;
        return node;
    }

    if (match(TokenType::LeftParen, nullptr)) {
        auto expr = parseExpression(errorMessage);
        if (!expr) {
            return nullptr;
        }
        if (!expect(TokenType::RightParen, nullptr, errorMessage)) {
            return nullptr;
        }
        return expr;
    }

    if (errorMessage) {
        if (currentToken().type == TokenType::End) {
            *errorMessage = QObject::tr("表达式不完整。");
        } else {
            const QString tokenText = currentToken().text.isEmpty() ? QObject::tr("<空>") : currentToken().text;
            *errorMessage = QObject::tr("无法解析表达式的该部分：%1").arg(tokenText);
        }
    }
    return nullptr;
}

double ExpressionEvaluator::evaluateNode(const Node *node, QString *errorMessage) const
{
    if (!node) {
        if (errorMessage) {
            *errorMessage = QObject::tr("表达式为空。");
        }
        return 0.0;
    }

    switch (node->type) {
    case Node::Type::Number:
        return node->value;
    case Node::Type::Variable: {
        const auto it = m_variables.constFind(node->text);
        if (it == m_variables.constEnd()) {
            if (errorMessage) {
                *errorMessage = QObject::tr("变量 %1 未赋值。").arg(node->text);
            }
            return 0.0;
        }
        return it.value();
    }
    case Node::Type::Operator: {
        const double leftValue = evaluateNode(node->left.get(), errorMessage);
        if (errorMessage && !errorMessage->isEmpty()) {
            return 0.0;
        }
        const double rightValue = evaluateNode(node->right.get(), errorMessage);
        if (errorMessage && !errorMessage->isEmpty()) {
            return 0.0;
        }

        const QChar op = node->text.isEmpty() ? QChar() : node->text.at(0);
        switch (op.unicode()) {
        case '+':
            return leftValue + rightValue;
        case '-':
            return leftValue - rightValue;
        case '*':
            return leftValue * rightValue;
        case '/':
            if (qFuzzyIsNull(rightValue)) {
                if (errorMessage) {
                    *errorMessage = QObject::tr("除数不能为0。");
                }
                return 0.0;
            }
            return leftValue / rightValue;
        default:
            if (errorMessage) {
                *errorMessage = QObject::tr("未知运算符：%1").arg(node->text);
            }
            return 0.0;
        }
    }
    }

    if (errorMessage) {
        *errorMessage = QObject::tr("未知的表达式节点类型。");
    }
    return 0.0;
}

bool ExpressionEvaluator::parseAssignmentLine(const QString &line, QString *errorMessage)
{
    const int equalIndex = line.indexOf(QLatin1Char('='));
    if (equalIndex <= 0) {
        if (errorMessage) {
            *errorMessage = QObject::tr("无效的赋值语句：%1").arg(line);
        }
        return false;
    }

    const QString name = line.left(equalIndex).trimmed();
    const QString valueText = line.mid(equalIndex + 1).trimmed();
    if (name.isEmpty() || valueText.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QObject::tr("无效的赋值语句：%1").arg(line);
        }
        return false;
    }

    if (!isIdentifierStart(name.at(0))) {
        if (errorMessage) {
            *errorMessage = QObject::tr("变量名无效：%1").arg(name);
        }
        return false;
    }
    for (int i = 1; i < name.size(); ++i) {
        if (!isIdentifierPart(name.at(i))) {
            if (errorMessage) {
                *errorMessage = QObject::tr("变量名无效：%1").arg(name);
            }
            return false;
        }
    }

    bool ok = false;
    const double value = valueText.toDouble(&ok);
    if (!ok) {
        if (errorMessage) {
            *errorMessage = QObject::tr("无法解析数值：%1").arg(valueText);
        }
        return false;
    }

    m_variables.insert(name, value);
    return true;
}
