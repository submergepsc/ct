#ifndef EXPRESSIONEVALUATOR_H
#define EXPRESSIONEVALUATOR_H

#include <memory>

#include <QList>
#include <QMap>
#include <QString>

class ExpressionEvaluator
{
public:
    struct Node
    {
        enum class Type { Number, Variable, Operator };

        Type type = Type::Number;
        QString text;
        double value = 0.0;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    bool ReadInput(const QString &expression, QString *errorMessage);
    bool Assign(const QString &assignments, QString *errorMessage);
    bool Evaluate(double *result, QString *errorMessage) const;

    const Node *root() const;

private:
    enum class TokenType { Number, Identifier, Operator, LeftParen, RightParen, End };

    struct Token
    {
        TokenType type = TokenType::End;
        QString text;
        double numberValue = 0.0;
    };

    bool tokenize(const QString &expression, QString *errorMessage);
    const Token &currentToken() const;
    bool match(TokenType type, Token *out = nullptr);
    bool expect(TokenType type, Token *out, QString *errorMessage);
    QChar ReadOperator(const Token &token) const;

    std::unique_ptr<Node> MakeExp(std::unique_ptr<Node> left, QChar op, std::unique_ptr<Node> right);
    std::unique_ptr<Node> parseExpression(QString *errorMessage);
    std::unique_ptr<Node> parseTerm(QString *errorMessage);
    std::unique_ptr<Node> parseFactor(QString *errorMessage);
    double evaluateNode(const Node *node, QString *errorMessage) const;
    bool parseAssignmentLine(const QString &line, QString *errorMessage);

    QList<Token> m_tokens;
    int m_position = 0;
    std::unique_ptr<Node> m_root;
    QMap<QString, double> m_variables;
};

#endif // EXPRESSIONEVALUATOR_H
