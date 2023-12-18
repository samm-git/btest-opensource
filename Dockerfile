# Use a minimal Alpine Linux base image with build tools
FROM alpine:latest

# Set the working directory inside the container
WORKDIR /app

# Install necessary build tools
RUN apk --no-cache add build-base

# Copy the build.sh script and btest source code into the container
COPY build.sh .
COPY . /app/

# Run the build.sh script to compile btest
RUN chmod +x build.sh && ./build.sh

# Define environment variables
ENV USERNAME admin
ENV PASSWORD admin

# Set the entry point for the container
ENTRYPOINT ["./btest"]

# Specify the default command to run when the container starts
CMD ["-a", "${USERNAME}", "-p", "${PASSWORD}", "-s"]
