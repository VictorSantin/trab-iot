from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# Variável global para armazenar os dados recebidos
dados = {
    "Temperatura": None,
    "Umidade": None
}

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/data', methods=['POST'])
def receber_dados():
    global dados
    content = request.get_json()
    if content:
        dados["Temperatura"] = content.get("Temperatura")
        dados["Umidade"] = content.get("Umidade")
        return jsonify({"status": "sucesso", "dados_recebidos": dados}), 200
    else:
        return jsonify({"status": "erro", "mensagem": "JSON inválido"}), 400

@app.route('/dados_json')
def dados_json():
    return jsonify(dados)

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True)

