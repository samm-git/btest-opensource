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
COPY --from=docker.io/tailscale/tailscale:stable /usr/local/bin/tailscaled /usr/local/bin/tailscaled
COPY --from=docker.io/tailscale/tailscale:stable /usr/local/bin/tailscale /usr/local/bin/tailscale
# Define environment variables
ENV USERNAME admin
ENV PASSWORD admin
RUN [ -n "$AUTHKEY" ] && [ ! -z "$AUTHKEY" ] && tailscale up --authkey $AUTHKEY || { echo "AuthKey is not set or empty. Exiting."; exit 1; }

# Set the entry point for the container
ENTRYPOINT ["./btest"]

# Specify the default command to run when the container starts
CMD ["-a", "${USERNAME}", "-p", "${PASSWORD}", "-s"]
