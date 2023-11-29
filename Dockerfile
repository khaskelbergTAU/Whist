FROM gradle:8.4-jdk-alpine AS build
COPY --chown=gradle:gradle 'Whist-Server' /home/gradle/src
COPY --chown=gradle:gradpe API/whist.h /home/gradle/src/src/main/resources/static/pages/whist.h
WORKDIR /home/gradle/src
RUN ./gradlew buildFatJar --no-daemon

FROM amazoncorretto:20-alpine
RUN mkdir /app
RUN apk add build-base
EXPOSE 80:8080
COPY API /app/API/
RUN g++ -I /app/API /app/API/grader.cpp /app/API/engine_api.c -o /app/API/grader
COPY Bots /tmp/bots-compile/source
RUN mkdir /tmp/bots-compile/exec
RUN gcc -I /app/API /tmp/bots-compile/source/min_bot.c /app/API/API.c -o /tmp/bots-compile/exec/min_bot
RUN gcc -I /app/API /tmp/bots-compile/source/max_bot.c /app/API/API.c -o /tmp/bots-compile/exec/max_bot
RUN gcc -I /app/API /tmp/bots-compile/source/random_bot.c /app/API/API.c -o /tmp/bots-compile/exec/random_bot
RUN cp /tmp/bots-compile/exec /app/serverFiles/bots/common/submissions
COPY --from=build /home/gradle/src/build/libs/*.jar /app/server.jar
ENTRYPOINT ["java", "-jar", "/app/server.jar"]