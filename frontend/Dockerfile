FROM node:20-alpine

WORKDIR /app

COPY package.json package-lock.json* ./
RUN npm install

COPY . .

ENV CHOKIDAR_USEPOLLING=true

CMD ["npm", "run", "dev", "--", "--host"]
